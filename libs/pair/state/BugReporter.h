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

private:
  virtual void checkEndBug(AbstractState& state) {
    for (auto& [var, val] : state) {
      if (this->isBugVar(var))
        continue;

      if (this->globals.locals.inSentinels(var) && !val.isFinal()) {
        auto varName = var->getName();
        auto fncName = this->getFunctionName();
        auto* bugData = new VolatileSentinelBug(varName, fncName);
        bugData->print(errs());
        this->addBugData(bugData);
      }
    }
  }

public:
  BugReporter(Globals& globals_, DfResults& dfResults_)
      : Base(globals_, dfResults_) {}

  void checkDoubleFlushBug(Variable* var, InstrInfo* ii, AbstractState& state,
                           const Context& context) {
    auto* instr = ii->getInstruction();
    auto srcLoc = DbgInstr::getSourceLocation(instr);
    
    if (this->isBugLoc(srcLoc))
      return;

    auto& val = state[var];
    if (val.isFlushed()) {
      auto ic = InstCntxt{instr, context};
      auto previc = this->getLastFlush(var, val);

      this->addBugLoc(srcLoc);

      auto varName = var->getName();

      auto cur = ic.getName();
      auto prev = previc.getName();
      auto* bugData = new DoubleFlushBug(varName, cur, prev);
      bugData->print(errs());
      this->addBugData(bugData);
    }
  }

  void checkCommitPairBug(Variable* var, InstrInfo* ii, AbstractState& state,
                          const Context& context) {
    auto* instr = ii->getInstruction();
    auto srcLoc = DbgInstr::getSourceLocation(instr);

    if (this->isBugLoc(srcLoc))
      return;

    for (auto* pair : var->getPairs()) {
      auto* otherVar = pair->getOther(var);

      if (this->isBugVar(otherVar))
        continue;

      auto& otherVal = state[otherVar];

      if (otherVal.isWriteFlush()) {
        auto ic = InstCntxt{instr, context};
        auto previc = this->getLastCommit(otherVar, otherVal);
        auto prevLoc = previc.getLoc();

        this->addBugLoc(srcLoc);
        this->addBugLoc(prevLoc);

        auto varName = var->getName();
        auto prevName = otherVar->getName();

        auto cur = ic.getName();
        auto prev = previc.getName();
        auto* bugData = new CommitPairBug(varName, prevName, cur, prev);
        bugData->print(errs());
        this->addBugData(bugData);
      }
    }
  }
};

} // namespace llvm