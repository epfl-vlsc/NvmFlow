

#include "AliasPass.h"
#include "Common.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/CFLAndersAliasAnalysis.h"
#include "llvm/Analysis/CFLSteensAliasAnalysis.h"

#include "analysis_util/AliasGroups.h"
#include "analysis_util/DfUtil.h"
#include "analysis_util/MemoryUtil.h"

#include <cassert>
#include <set>
using namespace std;
namespace llvm {

void AliasPass::print(raw_ostream& OS, const Module* m) const { OS << "pass\n"; }

void addToAgs(Function& F, AliasGroups& ags) {
  for (Instruction& I : instructions(F)) {
    if (auto* si = dyn_cast<StoreInst>(&I)) {
      errs() << "sol\n";
      ags.add(si);
    } else if (auto* ci = dyn_cast<CallInst>(&I)) {
      errs() << "yol\n";
      ags.add(ci);
      auto* f = ci->getCalledFunction();
      addToAgs(*f, ags);
    }
  }
}

void runOnFunction(Function& F, AAResults& AAR) {
  AliasGroups ags;
  addToAgs(F, ags);
  ags.createGroups(&AAR);
  ags.print(errs());
  errs() << "lol\n";
}

bool AliasPass::runOnModule(Module& M) {
  AAResults AAR(getAnalysis<TargetLibraryInfoWrapperPass>().getTLI());
  auto& cflResult = getAnalysis<CFLAndersAAWrapperPass>().getResult();
  AAR.addAAResult(cflResult);
  

  for (auto& F : M) {
    errs() << "lol\n";
    runOnFunction(F, AAR);
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