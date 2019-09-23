#pragma once
#include "Common.h"
#include "analysis_util/DfUtil.h"
#include "ds/Variable.h"
#include "Lattice.h"

namespace llvm {

template<typename Globals>
class Transfer {
  using AbstractState = std::map<Variable*, Lattice>;

  bool handlePfence(InstrInfo* ii, AbstractState& state) {
    bool stateChanged = false;

    for (auto& [var, val] : state) {
      if (val.isSclCommitWrite()) {
        val = Lattice::getVfence(val);
        stateChanged = true;
      }

      if (val.isDclCommitFlush()) {
        val = Lattice::getPfence(val);
        stateChanged = true;
      }
    }

    return stateChanged;
  }

  bool handleVfence(InstrInfo* ii, AbstractState& state) {
    bool stateChanged = false;
    for (auto& [var, val] : state) {
      if (val.isSclCommitWrite()) {
        val = Lattice::getVfence(val);
        stateChanged = true;
      }
    }

    return stateChanged;
  }

  void doFlush(Variable* var, AbstractState& state, bool useFence) {
    auto& val = state[var];

    val = Lattice::getDclFlushFlush(val);

    if (val.isDclCommitWrite()) {
      if (useFence) {
        val = Lattice::getCommitFence(val);
      } else {
        val = Lattice::getCommitFlush(val);
      }
    }
  }

  bool handleFlush(InstrInfo* ii, AbstractState& state, bool useFence) {
    auto* var = ii->getVariable();
    doFlush(var, state, useFence);

    for (auto* fvar : var->getFlushSet()) {
      doFlush(fvar, state, useFence);
    }

    return true;
  }

  void doWrite(Variable* var, AbstractState& state) {
    auto& val = state[var];
    val = Lattice::getWrite(val);
  }

  bool handleWrite(InstrInfo* ii, AbstractState& state) {
    auto* var = ii->getVariable();
    doWrite(var, state);

    for (auto* wvar : var->getWriteSet()) {
      doWrite(wvar, state);
    }

    return true;
  }

  Globals& globals;

public:
  Transfer(Module& M_, Globals& globals_) : globals(globals_) {}

  ~Transfer() {}

  void initLatticeValues(AbstractState& state) {
    // for tracking locals
    for (auto& var : globals.getVariables()) {
      auto* varPtr = (Variable*)&var;
      state[varPtr] = Lattice::getInit();
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