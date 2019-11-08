#pragma once
#include "Common.h"
#include "Lattice.h"
#include "analysis_util/DfUtil.h"
#include "ds/Variable.h"

namespace llvm {

template <typename Globals, typename BReporter> class Transfer {
  using AbstractState = std::map<Variable*, Lattice>;

  bool isLocal(InstrInfo* ii) {
    auto pv = ii->getParsedVarLhs();
    if (pv.isUsed() && pv.isObjPtrVar())
      return true;

    return false;
  }

  bool handlePfence(InstrInfo* ii, AbstractState& state, const Context& context) {
    auto instr = ii->getInstruction();
    bool stateChanged = false;

    for (auto& [var, val] : state) {
      if (val.isFlush()) {
        val = Lattice::getFence(val);
        stateChanged = true;
        breporter.addLastSeen(var, val, instr, context);
      }
    }

    return stateChanged;
  }

  bool handleFlush(InstrInfo* ii, AbstractState& state, bool useFence,
                   const Context& context) {
    auto* instr = ii->getInstruction();
    auto* var = ii->getVariableLhs();

    breporter.checkDoubleFlushBug(var, ii, state, context);

    auto& val = state[var];
    val = Lattice::getFlush(val, useFence);

    breporter.addLastSeen(var, val, instr, context);

    return true;
  }

  bool rhsIsNull(InstrInfo* ii) {
    if (ii->hasVariableRhs()) {
      auto pvRhs = ii->getParsedVarRhs();
      return pvRhs.isNull();
    }
    return false;
  }

  bool handleWrite(InstrInfo* ii, AbstractState& state,
                   const Context& context) {
    auto* instr = ii->getInstruction();
    auto* var = ii->getVariableLhs();

    bool isNull = rhsIsNull(ii);

    if (!isNull)
      breporter.checkCommitPtrBug(ii, state, context);

    if (isLocal(ii))
      return false;

    auto& val = state[var];
    if (isNull)
      val = Lattice::getFence(val);
    else
      val = Lattice::getWrite(val);

    breporter.addLastSeen(var, val, instr, context);

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
      // ignore scl case
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