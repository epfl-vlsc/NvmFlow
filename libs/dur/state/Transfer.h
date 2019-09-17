#pragma once
#include "Common.h"
#include "FlowTypes.h"
#include "analysis_util/DfUtil.h"
#include "ds/Globals.h"

namespace llvm {

class Transfer {
  bool handlePfence(InstrInfo* ii, AbstractState& state) {
    bool stateChanged = false;

    for (auto& [var, val] : state) {
      if (val.isDclCommitFlush()) {
        val = LatVal::getPfence(val);
        stateChanged = true;
      }
    }

    return stateChanged;
  }

  bool handleFlush(InstrInfo* ii, AbstractState& state, bool useFence) {
    auto* var = ii->getVariable();
    auto& val = state[var];

    val = LatVal::getDclFlushFlush(val);

    if (val.isDclCommitWrite()) {
      if (useFence) {
        val = LatVal::getCommitFence(val);
      } else {
        val = LatVal::getCommitFlush(val);
      }
    }

    return true;
  }

  bool handleWrite(InstrInfo* ii, AbstractState& state) {
    auto* var = ii->getVariable();
    auto& val = state[var];
    val = LatVal::getWrite(val);
    return true;
  }

  Globals& globals;

public:
  Transfer(Module& M_, Globals& globals_)
      : globals(globals_){}

  ~Transfer() {}

  void initLatticeValues(AbstractState& state) {
    // for tracking locals
    for (auto& var : globals.getVariables()) {
      auto* varPtr = (Variable*)&var;
      state[varPtr] = LatVal::getInit();
    }
  }

  bool handleInstruction(Instruction* i, AbstractState& state) {
    bool stateChanged = false;

    auto* ii = globals.locals.getInstrInfo(i);
    if (!ii)
      return stateChanged;

    switch (ii->getInstrType()) {
    case InstrInfo::WriteInstr:
      stateChanged = handleWrite(ii, state);
      break;
    case InstrInfo::FlushInstr:
      stateChanged = handleFlush(ii, state, false);
      break;
    case InstrInfo::FlushFenceInstr:
      stateChanged = handleFlush(ii, state, true);
      break;
    case InstrInfo::VfenceInstr:
      //ignore scl case
      break;
    case InstrInfo::PfenceInstr:
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