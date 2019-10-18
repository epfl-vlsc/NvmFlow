#include "AliasPass.h"
#include "Common.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/CFLAndersAliasAnalysis.h"
#include "llvm/Analysis/CFLSteensAliasAnalysis.h"

#include "llvm/Analysis/AliasSetTracker.h"

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

void aa(Module& M, AAResults& AAR) {

  for (auto& F : M) {
    
    for (auto& I : instructions(F)) {
      auto pv = InstrParser::parseVarLhs(&I);
      if (!pv.isUsed())
        continue;

      pv.print(errs());
    }
  }
}

bool AliasPass::runOnModule(Module& M) {
  AAResults AAR(getAnalysis<TargetLibraryInfoWrapperPass>().getTLI());
  // auto& cflResult = getAnalysis<CFLAndersAAWrapperPass>().getResult();
  auto& cflResult = getAnalysis<CFLSteensAAWrapperPass>().getResult();
  AAR.addAAResult(cflResult);

  aa(M, AAR);
  // printPV(M);
  // runAlias(M, AAR);
  // runAliasGroup(M, AAR);
  // runAliasSetTracker(M, AAR);
  // runAliasAnders(M, cflResult);

  return false;
}

void AliasPass::getAnalysisUsage(AnalysisUsage& AU) const {
  AU.addRequired<TargetLibraryInfoWrapperPass>();
  AU.addRequired<CFLAndersAAWrapperPass>();
  AU.addRequired<CFLSteensAAWrapperPass>();

  AU.setPreservesAll();
}

char AliasPass::ID = 0;
RegisterPass<AliasPass> X("alias", "Alias pass");

} // namespace llvm