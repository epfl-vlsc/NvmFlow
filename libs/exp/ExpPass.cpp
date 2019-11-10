#include "ExpPass.h"
#include "Common.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/CFLAndersAliasAnalysis.h"
#include "llvm/Analysis/CFLSteensAliasAnalysis.h"

#include "llvm/Analysis/AliasSetTracker.h"

#include "analysis_util/DfUtil.h"
#include "parser_util/AliasGroups.h"
#include "parser_util/InstrParser.h"

#include <cassert>
#include <set>
using namespace std;
namespace llvm {

void ExpPass::print(raw_ostream& OS, const Module* m) const { OS << "pass\n"; }

bool ExpPass::runOnModule(Module& M) {
  AAResults AAR(getAnalysis<TargetLibraryInfoWrapperPass>().getTLI());
  // auto& cflResult = getAnalysis<CFLAndersAAWrapperPass>().getResult();
  auto& cflResult = getAnalysis<CFLSteensAAWrapperPass>().getResult();
  AAR.addAAResult(cflResult);

  std::unordered_set<Value*> as;
  for (auto& F : M) {
    if (F.isIntrinsic() || F.isDeclaration())
      continue;

    if (!F.getName().equals("main") && !F.getName().contains("ipafnc"))
      continue;

    errs() << "function:" << F.getName() << "\n";
    for (auto& I : instructions(F)) {
      auto pv = InstrParser::parseVarLhs(&I);
      if (!pv.isUsed() || !pv.isPersistentVar())
        continue;
      pv.print(errs());
      auto* alias = pv.getObjAlias();
      errs() << *alias << "\n";
      as.insert(alias);

      errs() << I << "\n";
    }
  }

  std::unordered_set<Value*> seen;
  for (auto* p1 : as) {
    for (auto* p2 : as) {
      if(seen.count(p2))
        continue;
      auto r = AAR.alias(p1, p2);
      errs() << r << " " << *p1 << " " << *p2 << "\n";
    }
    seen.insert(p1);
  }
  return false;
}

void ExpPass::getAnalysisUsage(AnalysisUsage& AU) const {
  AU.addRequired<TargetLibraryInfoWrapperPass>();
  AU.addRequired<CFLAndersAAWrapperPass>();
  AU.addRequired<CFLSteensAAWrapperPass>();

  AU.setPreservesAll();
}

char ExpPass::ID = 0;
RegisterPass<ExpPass> X("exp", "Exp pass");

} // namespace llvm