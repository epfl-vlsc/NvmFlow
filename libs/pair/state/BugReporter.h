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
        this->addBugData(bugData);
      }
    }
  }

public:
  BugReporter(Globals& globals_, DfResults& dfResults_)
      : Base(globals_, dfResults_) {}

  void checkDoubleFlushBug(Variable* var, InstrInfo* ii, AbstractState& state) {
    if (this->isBugVar(var))
      return;

    auto& val = state[var];
    if (val.isFlushed()) {
      this->addBugVar(var);

      auto* instr = ii->getInstruction();
      auto* prevInstr = this->getLastFlush(var, val);

      auto varName = var->getName();
      auto srcLoc = DbgInstr::getSourceLocation(instr);
      auto prevLoc = DbgInstr::getSourceLocation(prevInstr);

      auto* bugData = new DoubleFlushBug(varName, srcLoc, prevLoc);
      this->addBugData(bugData);
    }
  }

  void checkCommitPairBug(Variable* var, InstrInfo* ii, AbstractState& state) {
    if (this->isBugVar(var))
      return;

    for (auto* pair : var->getPairs()) {
      auto* otherVar = pair->getOther(var);

      if (this->isBugVar(otherVar))
        continue;

      auto& otherVal = state[otherVar];

      if (otherVal.isWriteFlush()) {
        this->addBugVar(var);
        this->addBugVar(otherVar);

        auto* instr = ii->getInstruction();
        auto* otherInst = this->getLastCommit(otherVar, otherVal);

        auto varName = var->getName();
        auto prevName = otherVar->getName();
        auto srcLoc = DbgInstr::getSourceLocation(instr);
        auto prevLoc = DbgInstr::getSourceLocation(otherInst);

        auto* bugData = new CommitPairBug(varName, prevName, srcLoc, prevLoc);
        this->addBugData(bugData);
      }
    }
  }
};

} // namespace llvm