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
    auto* instr = ii->getInstruction();
    auto srcLoc = DbgInstr::getSourceLocation(instr);

    if (this->isBugLoc(srcLoc))
      return;

    auto& val = state[var];
    if (val.isFlushed()) {
      this->addBugLoc(srcLoc);

      auto* instr = ii->getInstruction();
      auto* prevInstr = this->getLastFlush(var, val);

      auto varName = var->getName();
      auto srcLoc = DbgInstr::getSourceLocation(instr);
      auto prevLoc = DbgInstr::getSourceLocation(prevInstr);

      auto* bugData = new DoubleFlushBug(varName, srcLoc, prevLoc);
      bugData->print(errs());
      this->addBugData(bugData);
    }
  }

  void checkCommitPtrBug(InstrInfo* ii, AbstractState& state) {
    auto* instr = ii->getInstruction();
    auto srcLoc = DbgInstr::getSourceLocation(instr);

    if (this->isBugLoc(srcLoc))
      return;

    auto pv = ii->getParsedVarLhs();
    if (!ii->hasVariableRhs() || !pv.isUsed() || !pv.isAnnotated())
      return;

    auto pvRhs = ii->getParsedVarRhs();
    if (pvRhs.isNull())
      return;

    auto* var = ii->getVariableRhs();

    auto& val = state[var];

    if (!val.isPersistent()) {
      this->addBugLoc(srcLoc);

      auto varName = var->getName();

      auto* bugData = new CommitPtrBug(varName, srcLoc);
      bugData->print(errs());
      this->addBugData(bugData);
    }
  }
};

} // namespace llvm