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

  void checkDoubleLogBug(Variable* var, InstrInfo* ii, AbstractState& state) {
    auto* instr = ii->getInstruction();
    auto srcLoc = DbgInstr::getSourceLocation(instr);

    if (this->isBugLoc(srcLoc))
      return;

    auto& val = state[var];
    if (val.isLogged()) {
      this->addBugLoc(srcLoc);

      auto* prevInstr = this->getLastFlush(var, val);
      auto varName = var->getName();

      auto prevLoc = DbgInstr::getSourceLocation(prevInstr);

      auto* bugData = new DoubleLogBug(varName, srcLoc, prevLoc);
      this->addBugData(bugData);
    }
  }

  void checkCommitBug(Variable* var, InstrInfo* ii, AbstractState& state) {
    auto* instr = ii->getInstruction();
    auto srcLoc = DbgInstr::getSourceLocation(instr);

    if (this->isBugLoc(srcLoc))
      return;

    auto& val = state[var];
    if (!val.isLog()) {
      this->addBugLoc(srcLoc);

      auto varName = var->getName();

      auto* bugData = new CommitBug(varName, srcLoc);
      this->addBugData(bugData);
    }
  }

  void checkOutTxBug(Variable* var, InstrInfo* ii, AbstractState& state) {
    auto* instr = ii->getInstruction();
    auto srcLoc = DbgInstr::getSourceLocation(instr);

    if (this->isBugLoc(srcLoc))
      return;

    auto& val = state[var];
    if (!val.inTx()) {
      this->addBugLoc(srcLoc);

      auto* accessedVar = ii->getVariable();
      auto varName = accessedVar->getName();
      

      auto* bugData = new OutTxBug(varName, srcLoc);
      this->addBugData(bugData);
    }
  }
};

} // namespace llvm