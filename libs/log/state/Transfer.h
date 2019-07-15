#pragma once
//#include "Report.h"
#include "Common.h"
#include "FlowTypes.h"

#include "analysis_util/DfUtil.h"

#include "ds/Units.h"

namespace llvm {

class Transfer {
private:
  bool handleLogging(InstructionInfo* ii, AbstractState& state) {
    auto* variable = ii->getVariable();
    assert(variable);

    if (variable->isField()) {
      // bug check
      state[variable] = LatVal::getLogged();
      return true;
    } else if (variable->isObj()) {
      auto* st = variable->getStType();
      bool stateChanged = false;
      for (auto* affectedVar : units.activeFunction->getAffectedVariables(st)) {
        // bug check
        state[affectedVar] = LatVal::getLogged();
        stateChanged = true;
      }
      return stateChanged;
    }

    return false;
  }

  bool handleWrite(InstructionInfo* ii, AbstractState& state) {
    // bug check
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

  /*
    // handle----------------------------------------------------
    bool handleWrite(StoreInst* si, LatVar var, AbstractState& state) {
      report.checkNotLoggedBug(si, var, state);
      return false;
    }

    bool handleLog(CallInst* ci, LatVar var, AbstractState& state) {
      report.checkDoubleLogBug(ci, var, state);

      state[var] = LatVal::getLogged();
      report.updateLastLocation(var, ci);

      return true;
    }

    bool handleTxBegin(CallInst* ci, AbstractState& state) {
      state[nullptr] = LatVal::getBeginTx(state[nullptr]);

      return true;
    }

    bool handleTxEnd(CallInst* ci, AbstractState& state) {
      state[nullptr] = LatVal::getEndTx(state[nullptr]);

      return true;
    }
   */
  Units& units;
  Variable* txLatVar;
  // Report& report;

public:
  Transfer(Module& M_, Units& units_) : units(units_) {
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