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

  void checkCommitPtrBug(InstrInfo* ii, AbstractState& state,
                         const Context& context) {
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
      auto ic = InstCntxt{instr, context};

      this->addBugLoc(srcLoc);

      auto varName = var->getName();

      auto cur = ic.getName();
      auto* bugData = new CommitPtrBug(varName, cur);
      bugData->print(errs());
      this->addBugData(bugData);
    }
  }
};

} // namespace llvm