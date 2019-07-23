#pragma once
#include "BugReporter.h"
#include "Common.h"
#include "FlowTypes.h"

#include "analysis_util/DfUtil.h"

#include "ds/Units.h"

namespace llvm {

class Transfer {
  bool handlePfence(InstructionInfo* ii, AbstractState& state) {
    bool stateChanged = false;
    for (auto& [var, val] : state) {
      if (val.isWriteScl()) {
        val = LatVal::getVfence(val);
        stateChanged = true;
      }

      if(val.isFlushDcl()){
        val = LatVal::getPfence(val);
        stateChanged = true;
      }
    }

    return stateChanged;
  }

  bool handleVfence(InstructionInfo* ii, AbstractState& state) {
    bool stateChanged = false;
    for (auto& [var, val] : state) {
      if (val.isWriteScl()) {
        val = LatVal::getVfence(val);
        stateChanged = true;
      }
    }

    return stateChanged;
  }

  bool handleFlushFence(InstructionInfo* ii, AbstractState& state) {
    auto* var = ii->getVariable();
    assert(var);

    auto& val = state[var];

    val = LatVal::getPfence(val);

    return true;
  }

  bool handleFlush(InstructionInfo* ii, AbstractState& state) {
    auto* var = ii->getVariable();
    assert(var);

    auto& val = state[var];
    
    val = LatVal::getFlush(val);

    return true;
  }

  bool handleWrite(InstructionInfo* ii, AbstractState& state) {
    auto* var = ii->getVariable();
    assert(var);

    auto& val = state[var];
    
    breporter.checkNotCommittedBug(ii, state);
    val = LatVal::getWrite(val);

    return true;
  }

  Units& units;
  BugReporter& breporter;

public:
  Transfer(Module& M_, Units& units_, BugReporter& breporter_)
      : units(units_), breporter(breporter_) {}

  ~Transfer() {}

  void initLatticeValues(AbstractState& state) {
    // for tracking variables
    for (auto& var : units.getVariables()) {
      state[var] = LatVal::getInit();
    }
  }

  bool handleInstruction(Instruction* i, AbstractState& state) {

    auto* ii = units.variables.getInstructionInfo(i);
    if (!ii)
      return false;

#ifdef DBGMODE
    errs() << "Analyze " << *i << "\n";
#endif

    switch (ii->iType) {
    case InstructionInfo::WriteInstr:
      return handleWrite(ii, state);
    case InstructionInfo::FlushInstr:
      return handleFlush(ii, state);
    case InstructionInfo::FlushFenceInstr:
      return handleFlushFence(ii, state);
    case InstructionInfo::VfenceInstr:
      return handleVfence(ii, state);
    case InstructionInfo::PfenceInstr:
      return handlePfence(ii, state);
    default:
      report_fatal_error("not correct instruction");
      return false;
    }
  }

}; // namespace llvm

} // namespace llvm