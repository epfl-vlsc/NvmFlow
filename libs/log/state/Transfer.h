#pragma once
#include "BugReporter.h"
#include "Common.h"
#include "FlowTypes.h"

#include "analysis_util/DfUtil.h"

#include "ds/Units.h"

namespace llvm {

class Transfer {
  void trackVar(Variable* var, InstructionInfo* ii) {
    breporter.updateLastLocation(var, ii);
  }

  bool doLog(Variable* var, AbstractState& state) {
    auto& val = state[var];

    if (!val.isLogged()) {
      val = LatVal::getLogged();
      return true;
    }
    return false;
  }

  bool handleLog(InstructionInfo* ii, AbstractState& state) {
    breporter.checkDoubleLogBug(txVar, ii, state);

    auto* var = ii->getVariable();
    assert(var);

    bool stateChanged = doLog(var, state);
    if (stateChanged)
      trackVar(var, ii);

    if (var->isObj()) {
      for (auto* field : units.variables.getFlushFields(var)) {
        bool fieldStateChanged = doLog(field, state);
        if (fieldStateChanged)
          trackVar(field, ii);

        stateChanged |= fieldStateChanged;
      }
    }

    return stateChanged;
  }

  bool handleWrite(InstructionInfo* ii, AbstractState& state) {
    breporter.checkNotCommittedBug(txVar, ii, state);
    return false;
  }

  bool handleTxBeg(InstructionInfo* ii, AbstractState& state) {
    state[txVar] = LatVal::getBeginTx(state[txVar]);
    return true;
  }

  bool handleTxEnd(InstructionInfo* ii, AbstractState& state) {
    state[txVar] = LatVal::getEndTx(state[txVar]);
    return true;
  }

  Units& units;
  BugReporter& breporter;
  Variable* txVar;

public:
  Transfer(Module& M_, Units& units_, BugReporter& breporter_)
      : units(units_), breporter(breporter_) {
    auto& llvmContext = M_.getContext();
    auto* st = StructType::create(llvmContext);
    txVar = new Variable(st, 0);
  }

  ~Transfer() { delete txVar; }

  void initLatticeValues(AbstractState& state) {
    // for tracking variables
    for (auto& var : units.getVariables()) {
      state[var] = LatVal::getInitLog();
    }

    // store transaction at nullptr
    state[txVar] = LatVal::getInitTx();
  }

  bool handleInstruction(Instruction* i, AbstractState& state) {
    bool stateChanged = false;

    auto* ii = units.variables.getInstructionInfo(i);
    if (!ii)
      return stateChanged;

    switch (ii->getInstrType()) {
    case InstructionInfo::LoggingInstr:
      stateChanged = handleLog(ii, state);
      break;
    case InstructionInfo::WriteInstr:
      stateChanged = handleWrite(ii, state);
      break;
    case InstructionInfo::TxBegInstr:
      stateChanged = handleTxBeg(ii, state);
      break;
    case InstructionInfo::TxEndInstr:
      stateChanged = handleTxEnd(ii, state);
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
}; // namespace llvm

} // namespace llvm