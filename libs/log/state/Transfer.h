#pragma once
#include "Common.h"
#include "Lattice.h"
#include "analysis_util/DfUtil.h"
#include "ds/Variable.h"

namespace llvm {

template <typename Globals, typename BReporter> class Transfer {
  using AbstractState = std::map<Variable*, Lattice>;

  void doLog(Variable* var, InstrInfo* ii, AbstractState& state,
             const Context& context) {
    auto* instr = ii->getInstruction();
    breporter.checkOutTxBug(txVar, ii, state, context);
    breporter.checkDoubleLogBug(var, ii, state, context);

    auto& val = state[var];

    val = Lattice::getLogged();

    breporter.addLastSeen(var, val, instr);
  }

  bool handleLog(InstrInfo* ii, AbstractState& state, const Context& context) {
    auto* var = ii->getVariable();
    doLog(var, ii, state, context);

    for (auto* fvar : var->getFlushSet()) {
      doLog(fvar, ii, state, context);
    }

    return true;
  }

  bool handleWrite(InstrInfo* ii, AbstractState& state,
                   const Context& context) {
    auto* var = ii->getVariable();
    breporter.checkOutTxBug(txVar, ii, state, context);
    breporter.checkCommitBug(var, ii, state, context);
    return true;
  }

  bool handleTxBeg(InstrInfo* ii, AbstractState& state) {
    auto& val = state[txVar];
    val = Lattice::getBeginTx(val);
    return true;
  }

  bool handleTxEnd(InstrInfo* ii, AbstractState& state) {
    auto& val = state[txVar];
    val = Lattice::getEndTx(val);
    return true;
  }

  Globals& globals;
  BReporter& breporter;
  Variable* txVar;

public:
  Transfer(Module& M_, Globals& globals_, BReporter& breporter_)
      : globals(globals_), breporter(breporter_) {
    auto& llvmContext = M_.getContext();
    auto* st = StructType::create(llvmContext, "TxVal");
    txVar = new Variable(st, 0);
  }

  ~Transfer() {}

  void initLatticeValues(AbstractState& state) {
    // for tracking variables
    for (auto& varRef : globals.getVariables()) {
      auto* var = (Variable*)&varRef;
      state[var] = Lattice::getInitLog();
    }

    // store transaction at nullptr
    state[txVar] = Lattice::getInitTx();
  }

  bool handleInstruction(Instruction* i, AbstractState& state,
                         const Context& context) {
    bool stateChanged = false;

    auto* ii = globals.locals.getInstrInfo(i);
    if (!ii)
      return stateChanged;

    switch (ii->getInstrType()) {
    case InstrInfo::LoggingInstr:
      stateChanged = handleLog(ii, state, context);
      break;
    case InstrInfo::WriteInstr:
      stateChanged = handleWrite(ii, state, context);
      break;
    case InstrInfo::TxBegInstr:
      stateChanged = handleTxBeg(ii, state);
      break;
    case InstrInfo::TxEndInstr:
      stateChanged = handleTxEnd(ii, state);
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
