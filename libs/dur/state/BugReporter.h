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

  void checkDoubleFlushBug(Variable* var, InstrInfo* ii, AbstractState& state) {
    if (this->isBugVar(var))
      return;

    auto& val = state[var];
    if (val.isFlushed()) {
      this->addBugVar(var);

      auto* instr = ii->getInstruction();
      auto* prevInstr = this->getLastSeen(var, val);

      auto varName = var->getName();
      auto srcLoc = DbgInstr::getSourceLocation(instr);
      auto prevLoc = DbgInstr::getSourceLocation(prevInstr);

      auto* bugData = new DoubleFlushBug(varName, srcLoc, prevLoc);
      this->addBugData(bugData);
    }
  }

  void checkCommitPtrBug(InstrInfo* ii, AbstractState& state) {
    auto* varInfo = ii->getVarInfo();
    if (!varInfo->isAnnotated() || !ii->hasVariableRhs())
      return;

    auto* var = ii->getVariableRhs();
    if (this->isBugVar(var))
      return;

    auto& val = state[var];
    if (!val.isFence()) {
      this->addBugVar(var);

      auto* instr = ii->getInstruction();

      auto varName = varInfo->getName();
      auto srcLoc = DbgInstr::getSourceLocation(instr);

      auto* bugData = new CommitPtrBug(varName, srcLoc);
      this->addBugData(bugData);
    }
  }
};

} // namespace llvm