#pragma once
#include "Common.h"
#include "Lattice.h"
#include "analysis_util/DfUtil.h"
#include "ds/Variable.h"

namespace llvm {

template <typename Globals, typename BReporter> class Transfer {
  using AbstractState = std::map<Variable*, Lattice>;

  bool handlePfence(InstrInfo* ii, AbstractState& state) {
    auto instr = ii->getInstruction();
    bool stateChanged = false;

    for (auto& [var, val] : state) {
      if (val.isFlush()) {
        val = Lattice::getFence(val);
        stateChanged = true;
        breporter.addLastSeen(var, val, instr);
      }
    }

    return stateChanged;
  }

  void doFlush(Variable* var, InstrInfo* ii, AbstractState& state,
               bool useFence) {
    auto* instr = ii->getInstruction();
    breporter.checkDoubleFlushBug(var, ii, state);

    auto& val = state[var];

    val = Lattice::getFlush(val, useFence);

    breporter.addLastSeen(var, val, instr);
  }

  bool handleFlush(InstrInfo* ii, AbstractState& state, bool useFence) {
    auto* var = ii->getVariable();
    doFlush(var, ii, state, useFence);

    for (auto* fvar : var->getFlushSet()) {
      doFlush(fvar, ii, state, useFence);
    }

    return true;
  }

  void doWrite(Variable* var, InstrInfo* ii, AbstractState& state) {
    auto* instr = ii->getInstruction();
    breporter.checkCommitPairBug(var, ii, state);

    auto& val = state[var];
    val = Lattice::getWrite(val);

    breporter.addLastSeen(var, val, instr);
  }

  bool handleWrite(InstrInfo* ii, AbstractState& state) {
    auto* var = ii->getVariable();
    doWrite(var, ii, state);

    for (auto* wvar : var->getWriteSet()) {
      doWrite(wvar, ii, state);
    }

    return true;
  }

  Globals& globals;
  BReporter& breporter;

public:
  Transfer(Module& M_, Globals& globals_, BReporter& breporter_)
      : globals(globals_), breporter(breporter_) {}

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
      // stateChanged = handleVfence(ii, state);
      break;
    case InstrInfo::PfenceInstr:
      stateChanged = handlePfence(ii, state);
      break;
    default:
      report_fatal_error("not correct instruction");
    }

#ifdef DBGMODE
    errs() << "Analyze "; 
    ii->print(errs());
    if (stateChanged)
      printState(state);
#endif

    return stateChanged;
  }
};

} // namespace llvm