

#include "AliasPass.h"
#include "Common.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/CFLAndersAliasAnalysis.h"
#include "llvm/Analysis/CFLSteensAliasAnalysis.h"

#include "analysis_util/AliasGroups.h"
#include "analysis_util/DfUtil.h"
#include "parser_util/InstrParser.h"

#include <cassert>
#include <set>
using namespace std;
namespace llvm {

void AliasPass::print(raw_ostream& OS, const Module* m) const {
  OS << "pass\n";
}

/*
std::set<Value*> vals;
  for (auto& I : instructions(F)) {
    if (auto* si = dyn_cast<StoreInst>(&I)) {
      auto* ptrOpnd = si->getPointerOperand();
      auto* valOpnd = si->getValueOperand();

      auto* ptrType = ptrOpnd->getType();
      auto* valType = valOpnd->getType();

      errs() << *si << "\n";
      errs() << "\t" << *ptrOpnd << " ";
      ptrType->print(errs());
      errs() << "\n";

      errs() << "\t" << *valOpnd << " ";
      valType->print(errs());
      errs() << "\n";

      if (ptrType->isPointerTy())
        vals.insert(ptrOpnd);

      if (valType->isPointerTy())
        vals.insert(valOpnd);

      valType->print(errs());
    } else if (auto* ci = dyn_cast<CallInst>(&I)) {
      if (isa<DbgValueInst>(ci))
        continue;

      auto* ptrOpnd = ci->getArgOperand(0);

      auto* ptrType = ptrOpnd->getType();

      errs() << *ci << "\n";
      errs() << "\t" << *ptrOpnd << " ";
      ptrType->print(errs());
      errs() << "\n";

      if (ptrType->isPointerTy())
        vals.insert(ptrOpnd);

      ptrType->print(errs());
    } else if (auto* li = dyn_cast<LoadInst>(&I)) {
      auto* ptrOpnd = li->getPointerOperand();

      auto* ptrType = ptrOpnd->getType();

      errs() << *li << "\n";
      errs() << "\t" << *ptrOpnd << " ";
      ptrType->print(errs());
      errs() << "\n";

      if (ptrType->isPointerTy())
        vals.insert(ptrOpnd);
    }
  }
  for (auto& v1 : vals) {
    for (auto& v2 : vals) {
      auto res = AAR.alias(v1, v2);
      errs() << res << " " << *v1 << " " << *v2 << "\n";
    }
  }
*/

bool AliasPass::runOnModule(Module& M) {
  AAResults AAR(getAnalysis<TargetLibraryInfoWrapperPass>().getTLI());
  auto& cflResult = getAnalysis<CFLAndersAAWrapperPass>().getResult();
  AAR.addAAResult(cflResult);

  AliasGroups ag(AAR);

  for (auto& F : M) {
    for (auto& I : instructions(F)) {
      auto pv = InstrParser::parseInstruction(&I);
      if (!pv.isUsed())
        continue;

      auto* lhs = pv.getOpndVar();
      auto* lv = pv.getLocalVar();
      ag.insert(lhs, lv);

      if (auto* rhs = pv.getRhs()) {
        ag.insert(rhs);
      }
    }
  }
  ag.print(errs());

  for (auto& F : M) {
    for (auto& I : instructions(F)) {
      auto pv = InstrParser::parseInstruction(&I);
      if (!pv.isUsed())
        continue;

      auto* lhs = pv.getOpndVar();
      int setNo = ag.getAliasSetNo(lhs);
      errs() << *lhs << " " << setNo << "\n";
      if (auto* rhs = pv.getRhs()) {
        int setNo = ag.getAliasSetNo(rhs);
        errs() << *rhs << " " << setNo << "\n";
      }
    }
  }

  return false;
}

void AliasPass::getAnalysisUsage(AnalysisUsage& AU) const {
  AU.addRequired<TargetLibraryInfoWrapperPass>();
  AU.addRequired<CFLAndersAAWrapperPass>();

  AU.setPreservesAll();
}

char AliasPass::ID = 0;
RegisterPass<AliasPass> X("alias", "Alias pass");

} // namespace llvm