#pragma once
#include "BugReporter.h"
#include "Common.h"
#include "FlowTypes.h"

#include "analysis_util/DfUtil.h"

#include "ds/Globals.h"

namespace llvm {

class Transfer {
  void trackVar(Variable* var, InstrInfo* ii) {
    breporter.updateLastLocation(var, ii);
  }

  bool handlePfence(InstrInfo* ii, AbstractState& state) {
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

  bool handleVfence(InstrInfo* ii, AbstractState& state) {
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
        val = LatVal::getCommitFence(val);
      } else {
        val = LatVal::getCommitFlush(val);
      }
      stateChanged = true;
    }

    return stateChanged;
  }

  bool handleFlush(InstrInfo* ii, AbstractState& state, bool useFence) {
    breporter.checkDoubleFlushBug(ii, state);

    auto* var = ii->getVariable();
    assert(var);

    bool stateChanged = doFlush(var, state, useFence);
    if (stateChanged)
      trackVar(var, ii);

    for (auto* fvar : var->getFlushSet()) {
      bool fieldStateChanged = doFlush(fvar, state, useFence);
      if (fieldStateChanged)
        trackVar(fvar, ii);

      stateChanged |= fieldStateChanged;
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

  bool handleWrite(InstrInfo* ii, AbstractState& state) {
    breporter.checkNotCommittedBug(ii, state);

    auto* var = ii->getVariable();
    assert(var);

    bool stateChanged = doWrite(var, state);
    if (stateChanged)
      trackVar(var, ii);

    for (auto* wvar : var->getWriteSet()) {
      bool objStateChanged = doWrite(wvar, state);
      if (objStateChanged)
        trackVar(wvar, ii);

      stateChanged |= objStateChanged;
    }

    return stateChanged;
  }

  Globals& globals;
  BugReporter& breporter;

public:
  Transfer(Module& M_, Globals& globals_, BugReporter& breporter_)
      : globals(globals_), breporter(breporter_) {}

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
      stateChanged = handleVfence(ii, state);
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