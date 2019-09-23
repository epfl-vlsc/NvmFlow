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

  bool handleFlush(InstrInfo* ii, AbstractState& state, bool useFence) {
    auto* instr = ii->getInstruction();
    auto* var = ii->getVariable();

    breporter.checkDoubleFlushBug(var, ii, state);

    auto& val = state[var];

    val = Lattice::getFlush(val, useFence);

    breporter.addLastSeen(var, val, instr);

    return true;
  }

  bool handleWrite(InstrInfo* ii, AbstractState& state) {
    auto* instr = ii->getInstruction();

    breporter.checkCommitPtrBug(ii, state);

    auto* var = ii->getVariable();
    auto& val = state[var];
    val = Lattice::getWrite(val);

    breporter.addLastSeen(var, val, instr);

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
      // ignore scl case
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