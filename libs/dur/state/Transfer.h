#pragma once
#include "BugReporter.h"
#include "Common.h"
#include "FlowTypes.h"

#include "analysis_util/DfUtil.h"

#include "ds/Units.h"

namespace llvm {

class Transfer {
  bool handlePfence(InstructionInfo* ii, AbstractState& state) {
    bool stateChanged = false;

    for (auto& [var, val] : state) {
      if (val.isFlush()) {
        val = LatVal::getFence(val);
        stateChanged = true;
      }
    }

    return stateChanged;
  }

  bool doFlushFence(Variable* var, AbstractState& state) {
    auto& val = state[var];

    if (val.isWrite()) {
      val = LatVal::getFence(val);
      return true;
    }

    return false;
  }

  bool doNormalFlush(Variable* var, AbstractState& state) {
    auto& val = state[var];

    if (val.isWrite()) {
      val = LatVal::getFlush(val);
      return true;
    }

    return false;
  }

  bool doFlush(Variable* var, AbstractState& state, bool useFence) {
    if (!useFence)
      return doNormalFlush(var, state);
    else
      return doFlushFence(var, state);
  }

  bool handleFlush(InstructionInfo* ii, AbstractState& state, bool useFence) {
    auto* var = ii->getVariable();
    assert(var);
   
    return doFlush(var, state, useFence);
  }

  bool handleWrite(InstructionInfo* ii, AbstractState& state) {
    breporter.checkNotCommittedBug(ii, state);

    auto* var = ii->getVariable();
    assert(var);

    // todo find var
    auto& val = state[var];

    if (val.isWrite())
      return false;

    val = LatVal::getWrite(val);
    return true;
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
#endif

    switch (ii->iType) {
    case InstructionInfo::WriteInstr:
      return handleWrite(ii, state);
    case InstructionInfo::FlushInstr:
      return handleFlush(ii, state, false);
    case InstructionInfo::FlushFenceInstr:
      return handleFlush(ii, state, true);
    case InstructionInfo::PfenceInstr:
      return handlePfence(ii, state);
    default:
      report_fatal_error("not correct instruction");
      return false;
    }
  }
};

} // namespace llvm