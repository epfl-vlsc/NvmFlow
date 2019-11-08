#pragma once
#include "Common.h"

#include "BugData.h"
#include "Lattice.h"
#include "analysis_util/BugUtil.h"
#include "ds/InstrInfo.h"
#include "ds/Variable.h"

namespace llvm {

template <typename Globals>
class BugReporter : public BugUtil<Globals, Variable*, Lattice> {
public:
  using Base = BugUtil<Globals, Variable*, Lattice>;
  using AbstractState = typename Base::AbstractState;
  using DfResults = typename Base::DfResults;

public:
  BugReporter(Globals& globals_, DfResults& dfResults_)
      : Base(globals_, dfResults_) {}

  void checkDoubleLogBug(Variable* var, InstrInfo* ii, AbstractState& state,
                         const Context& context) {
    auto* instr = ii->getInstruction();
    auto srcLoc = DbgInstr::getSourceLocation(instr);

    if (this->isBugLoc(srcLoc))
      return;

    auto& val = state[var];
    if (val.isLogged()) {
      auto ic = InstCntxt{instr, context};
      auto previc = this->getLastFlush(var, val);

      this->addBugLoc(srcLoc);

      auto varName = var->getName();

      auto cur = ic.getName();
      auto prev = previc.getName();
      auto* bugData = new DoubleLogBug(varName, cur, prev);
      bugData->print(errs());
      this->addBugData(bugData);
    }
  }

  void checkCommitBug(Variable* var, InstrInfo* ii, AbstractState& state,
                      const Context& context) {
    auto* instr = ii->getInstruction();
    auto srcLoc = DbgInstr::getSourceLocation(instr);

    if (this->isBugLoc(srcLoc))
      return;

    auto& val = state[var];
    if (!val.isLog()) {
      auto ic = InstCntxt{instr, context};

      this->addBugLoc(srcLoc);

      auto varName = var->getName();

      auto cur = ic.getName();
      auto* bugData = new CommitBug(varName, cur);
      bugData->print(errs());
      this->addBugData(bugData);
    }
  }

  void checkOutTxBug(Variable* var, InstrInfo* ii, AbstractState& state,
                     const Context& context) {
    auto* instr = ii->getInstruction();
    auto srcLoc = DbgInstr::getSourceLocation(instr);

    if (this->isBugLoc(srcLoc))
      return;

    auto& val = state[var];
    if (!val.inTx()) {
      auto ic = InstCntxt{instr, context};

      this->addBugLoc(srcLoc);

      auto* accessedVar = ii->getVariable();
      auto varName = accessedVar->getName();

      auto cur = ic.getName();
      auto* bugData = new OutTxBug(varName, cur);
      bugData->print(errs());
      this->addBugData(bugData);
    }
  }
};

} // namespace llvm