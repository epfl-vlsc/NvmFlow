#pragma once
#include "Common.h"
#include "analysis_util/DfUtil.h"
#include "analysis_util/PersistLattice.h"
#include "ds/Variable.h"

namespace llvm {

template <typename Globals, typename BReporter> class Transfer {
  using AbstractState = std::map<Variable*, Lattice>;

  bool handlePfence(InstrInfo* ii, AbstractState& state,
                    const Context& context) {
    auto instr = ii->getInstruction();
    bool stateChanged = false;

    for (auto& [var, val] : state) {
      if (val.isFlush()) {
        val = Lattice::getFence(val, instr, context);
        stateChanged = true;
      }
    }

    return stateChanged;
  }

  void doFlush(Variable* var, InstrInfo* ii, AbstractState& state,
               bool useFence, const Context& context) {
    auto* instr = ii->getInstruction();
    breporter.checkDoubleFlushBug(var, ii, state, context);

    auto& val = state[var];

    val = Lattice::getFlush(val, useFence, instr, context);
  }

  bool handleFlush(InstrInfo* ii, AbstractState& state, bool useFence,
                   const Context& context) {
    auto* var = ii->getVariable();
    doFlush(var, ii, state, useFence, context);

    for (auto* fvar : var->getFlushSet()) {
      doFlush(fvar, ii, state, useFence, context);
    }

    return true;
  }

  void doWrite(Variable* var, InstrInfo* ii, AbstractState& state,
               const Context& context) {
    auto* instr = ii->getInstruction();
    breporter.checkCommitPairBug(var, ii, state, context);

    auto& val = state[var];
    val = Lattice::getWrite(val, instr, context);
  }

  bool handleWrite(InstrInfo* ii, AbstractState& state,
                   const Context& context) {
    auto* var = ii->getVariable();
    doWrite(var, ii, state, context);

    for (auto* wvar : var->getWriteSet()) {
      doWrite(wvar, ii, state, context);
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

  bool handleInstruction(Instruction* i, AbstractState& state,
                         const Context& context) {
    bool stateChanged = false;

    auto* ii = globals.locals.getInstrInfo(i);
    if (!ii)
      return stateChanged;

    switch (ii->getInstrType()) {
    case InstrInfo::WriteInstr:
      stateChanged = handleWrite(ii, state, context);
      break;
    case InstrInfo::FlushInstr:
      stateChanged = handleFlush(ii, state, false, context);
      break;
    case InstrInfo::FlushFenceInstr:
      stateChanged = handleFlush(ii, state, true, context);
      break;
    case InstrInfo::VfenceInstr:
      // stateChanged = handleVfence(ii, state);
      break;
    case InstrInfo::PfenceInstr:
      stateChanged = handlePfence(ii, state, context);
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