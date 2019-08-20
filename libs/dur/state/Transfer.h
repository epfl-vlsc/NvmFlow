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

  bool handleFlush(InstructionInfo* ii, AbstractState& state, bool useFence) {
    bool stateChanged = false;
    auto* var = ii->getVariable();
    auto* ag = var->getAliasGroup();

    auto& val = state[ag];

    if (val.isFlush() && !useFence)
      return false;
    else if (val.isFence() && useFence)
      return false;

    //do flush
    if (useFence) {
      val = LatVal::getFence(val);
    } else {
      val = LatVal::getFlush(val);
    }
    return true;
  }

  bool handleWrite(InstructionInfo* ii, AbstractState& state) {
    breporter.checkNotCommittedBug(ii, state);

    auto* var = ii->getVariable();
    auto* ag = var->getAliasGroup();

    auto& val = state[ag];

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
    bool stateChanged = false;

    auto* ii = units.variables.getInstructionInfo(i);
    if (!ii)
      return stateChanged;

    switch (ii->getInstrType()) {
    case InstructionInfo::WriteInstr:
      stateChanged = handleWrite(ii, state);
      break;
    case InstructionInfo::FlushInstr:
      stateChanged = handleFlush(ii, state, false);
      break;
    case InstructionInfo::FlushFenceInstr:
      stateChanged = handleFlush(ii, state, true);
      break;
    case InstructionInfo::PfenceInstr:
      stateChanged = handlePfence(ii, state);
      break;
    default:
      report_fatal_error("not correct instruction");
      return false;
    }

#ifdef DBGMODE
    errs() << "Analyze " << DbgInstr::getSourceLocation(i) << "\n";
    if (stateChanged)
      printState(state);
#endif

    return stateChanged;
  }
};

} // namespace llvm