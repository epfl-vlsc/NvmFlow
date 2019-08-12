#pragma once
#include "BugReporter.h"
#include "Common.h"
#include "FlowTypes.h"

#include "analysis_util/DfUtil.h"

#include "ds/Units.h"

namespace llvm {

class Transfer {
  void trackVar(Variable* var, InstructionInfo* ii) {
    breporter.updateLastLocation(var, ii);
  }

  bool handlePfence(InstructionInfo* ii, AbstractState& state) {
    bool stateChanged = false;

    for (auto& [var, val] : state) {
      if (val.isSclCommitWrite()) {
        val = LatVal::getVfence(val);
        stateChanged = true;
      }

      if (val.isDclCommitFlush()) {
        val = LatVal::getPfence(val);
        stateChanged = true;
      }
    }

    return stateChanged;
  }

  bool handleVfence(InstructionInfo* ii, AbstractState& state) {
    bool stateChanged = false;
    for (auto& [var, val] : state) {
      if (val.isSclCommitWrite()) {
        val = LatVal::getVfence(val);
        stateChanged = true;
      }
    }

    return stateChanged;
  }

  bool doFlush(Variable* var, AbstractState& state, bool useFence) {
    bool stateChanged = false;

    auto& val = state[var];

    if (!val.isDclFlushFlush()) {
      val = LatVal::getDclFlushFlush(val);
      stateChanged = true;
    }

    if (val.isDclCommitWrite()) {
      if (useFence) {
        val = LatVal::getCommitFlush(val);
      } else {
        val = LatVal::getCommitFence(val);
      }
      stateChanged = true;
    }

    return stateChanged;
  }

  bool handleFlush(InstructionInfo* ii, AbstractState& state, bool useFence) {
    breporter.checkDoubleFlushBug(ii, state);

    auto* var = ii->getVariable();
    assert(var);

    bool stateChanged = doFlush(var, state, useFence);
    if (stateChanged)
      trackVar(var, ii);

    if (var->isObj()) {
      for (auto* field : units.variables.getFlushFields(var)) {
        bool fieldStateChanged = doFlush(field, state, useFence);
        if (fieldStateChanged)
          trackVar(field, ii);

        stateChanged |= fieldStateChanged;
      }
    }

    return stateChanged;
  }

  bool doWrite(Variable* var, AbstractState& state) {
    auto& val = state[var];

    if (val.isWrite())
      return false;

    val = LatVal::getWrite(val);
    return true;
  }

  bool handleWrite(InstructionInfo* ii, AbstractState& state) {
    breporter.checkNotCommittedBug(ii, state);

    auto* var = ii->getVariable();
    assert(var);

    bool stateChanged = doWrite(var, state);
    if (stateChanged)
      trackVar(var, ii);

    if (var->isField()) {
      for (auto* obj : units.variables.getWriteObjs(var)) {
        bool objStateChanged = doWrite(obj, state);
        if (objStateChanged)
          trackVar(obj, ii);

        stateChanged |= objStateChanged;
      }
    }

    return stateChanged;
  }

  Units& units;
  BugReporter& breporter;

public:
  Transfer(Module& M_, Units& units_, BugReporter& breporter_)
      : units(units_), breporter(breporter_) {}

  ~Transfer() {}

  void initLatticeValues(AbstractState& state) {
    // for tracking variables
    for (auto& var : units.getVariables()) {
      state[var] = LatVal::getInit();
    }
  }

  bool handleInstruction(Instruction* i, AbstractState& state) {
    auto* ii = units.variables.getInstructionInfo(i);
    if (!ii)
      return false;

#ifdef DBGMODE
    errs() << "Analyze " << DbgInstr::getSourceLocation(i) << "\n";
    printState(state);
#endif

    switch (ii->getInstrType()) {
    case InstructionInfo::WriteInstr:
      return handleWrite(ii, state);
    case InstructionInfo::FlushInstr:
      return handleFlush(ii, state, false);
    case InstructionInfo::FlushFenceInstr:
      return handleFlush(ii, state, true);
    case InstructionInfo::VfenceInstr:
      return handleVfence(ii, state);
    case InstructionInfo::PfenceInstr:
      return handlePfence(ii, state);
    default:
      report_fatal_error("not correct instruction");
      return false;
    }
  }
};

} // namespace llvm