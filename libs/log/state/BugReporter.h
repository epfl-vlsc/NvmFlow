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
      this->addBugLoc(srcLoc);

      auto varName = var->getName();

      auto curLoc = context.getFullName(srcLoc);
      auto prevLoc = val.getFlushInfo();
      auto* bugData = new DoubleLogBug(varName, curLoc, prevLoc);
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
      this->addBugLoc(srcLoc);

      auto varName = var->getName();

      auto curLoc = context.getFullName(srcLoc);
      auto* bugData = new CommitBug(varName, curLoc);
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
      this->addBugLoc(srcLoc);

      auto* accessedVar = ii->getVariable();
      auto varName = accessedVar->getName();

      auto curLoc = context.getFullName(srcLoc);
      auto* bugData = new OutTxBug(varName, curLoc);
      bugData->print(errs());
      this->addBugData(bugData);
    }
  }
};

} // namespace llvm