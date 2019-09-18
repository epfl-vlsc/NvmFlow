#pragma once
#include "Common.h"
#include "analysis_util/DfUtil.h"
#include "ds/Variable.h"
#include "Lattice.h"

namespace llvm {

template<typename Globals, typename LatVar, typename LatVal>
class Transfer {
  using AbstractState = std::map<LatVar, LatVal>;

  bool handlePfence(InstrInfo* ii, AbstractState& state) {
    errs() << "handle fence\n";
    bool stateChanged = false;

    for (auto& [var, val] : state) {
      if (val.isDclCommitFlush()) {
        val = LatVal::getPfence(val);
        stateChanged = true;
      }
    }

    return stateChanged;
  }

  bool handleFlush(InstrInfo* ii, AbstractState& state, bool useFence) {
    errs() << "handle flush\n";
    auto* var = ii->getVariable();
    auto& val = state[var];

    val = LatVal::getFlush(val, useFence);

    return true;
  }

  bool handleWrite(InstrInfo* ii, AbstractState& state) {
    errs() << "handle write\n";
    auto* varInfo = ii->getVarInfo();
    if(varInfo->isAnnotated())
      return false;

    auto* var = ii->getVariable();
    auto& val = state[var];
    val = LatVal::getWrite(val);
    return true;
  }

  Globals& globals;

public:
  Transfer(Module& M_, Globals& globals_)
      : globals(globals_){}

  ~Transfer() {}

  void initLatticeValues(AbstractState& state) {
    // for tracking locals
    for (auto& var : globals.getVariables()) {
      auto* varPtr = (Variable*)&var;
      state[varPtr] = LatVal::getInit();
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
      //ignore scl case
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