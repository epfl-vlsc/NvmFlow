#pragma once
#include "BugReporter.h"
#include "Common.h"
#include "FlowTypes.h"

#include "analysis_util/DfUtil.h"

#include "ds/Units.h"

namespace llvm {

class Transfer {
private:
  bool handleLogging(InstructionInfo* ii, AbstractState& state) {
    auto* variable = ii->getVariable();
    auto* instruction = ii->getInstruction();
    assert(variable);

    if (variable->isField()) {
      breporter.checkDoubleLogBug(instruction, variable, txLatVar, state);
      state[variable] = LatVal::getLogged();
      return true;
    } else if (variable->isObj()) {
      auto* st = variable->getStType();
      bool stateChanged = false;
      for (auto* affectedVar : units.activeFunction->getAffectedVariables(st)) {
        breporter.checkDoubleLogBug(instruction, affectedVar, txLatVar, state);
        state[affectedVar] = LatVal::getLogged();
        stateChanged = true;
      }
      return stateChanged;
    }

    return false;
  }

  bool handleWrite(InstructionInfo* ii, AbstractState& state) {
    auto* variable = ii->getVariable();
    auto* instruction = ii->getInstruction();
    assert(variable);

    breporter.checkNotLoggedBug(instruction, variable, txLatVar, state);
    return false;
  }

  bool handleTxBeg(InstructionInfo* ii, AbstractState& state) {
    state[txLatVar] = LatVal::getBeginTx(state[txLatVar]);
    return true;
  }

  bool handleTxEnd(InstructionInfo* ii, AbstractState& state) {
    state[txLatVar] = LatVal::getBeginTx(state[txLatVar]);
    return true;
  }

  Units& units;
  BugReporter& breporter;
  Variable* txLatVar;

public:
  Transfer(Module& M_, Units& units_, BugReporter& breporter_)
      : units(units_), breporter(breporter_) {
    auto& llvmContext = M_.getContext();
    auto* st = StructType::create(llvmContext);
    txLatVar = new Variable(st, 0);
  }

  ~Transfer() { delete txLatVar; }

  void initLatticeValues(AbstractState& state) {
    // for tracking variables
    for (auto& var : units.getVariables()) {
      state[var] = LatVal::getInitLog();
    }

    // store transaction at nullptr
    state[txLatVar] = LatVal::getInitTx();
  }

  bool handleInstruction(Instruction* i, AbstractState& state) {
    auto* ii = units.activeFunction->getInstructionInfo(i);
    if (!ii)
      return false;

    switch (ii->iType) {
    case InstructionInfo::LoggingInstr:
      return handleLogging(ii, state);
    case InstructionInfo::WriteInstr:
      return handleWrite(ii, state);
    case InstructionInfo::TxBegInstr:
      return handleTxBeg(ii, state);
    case InstructionInfo::TxEndInstr:
      return handleTxEnd(ii, state);
    default:
      report_fatal_error("not correct instruction");
      return false;
    }
  }
};

} // namespace llvm