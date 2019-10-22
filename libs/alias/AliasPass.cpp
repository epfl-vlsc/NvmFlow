#include "AliasPass.h"
#include "AliasUtils.h"

using namespace std;
namespace llvm {

void AliasPass::print(raw_ostream& OS, const Module* m) const {
  OS << "pass\n";
}

bool AliasPass::runOnModule(Module& M) {

  /*
  AAResults AAR(getAnalysis<TargetLibraryInfoWrapperPass>().getTLI());
  // auto& cflResult = getAnalysis<CFLAndersAAWrapperPass>().getResult();
  auto& cflResult = getAnalysis<CFLSteensAAWrapperPass>().getResult();
  AAR.addAAResult(cflResult);

  AA a(M, AAR);
  // printPV(M);
  // runAlias(M, AAR);
  // runAliasGroup(M, AAR);
  // runAliasSetTracker(M, AAR);
  // runAliasAnders(M, cflResult);
  */

  auto& TLI = getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();
  AAResults AAR(TLI);
  PointsTo pt(TLI);
  auto& steens = pt.getResult();
  AA a(M, AAR);

  return false;
}

void AliasPass::getAnalysisUsage(AnalysisUsage& AU) const {
  AU.addRequired<TargetLibraryInfoWrapperPass>();
  AU.setPreservesAll();
}

char AliasPass::ID = 0;
RegisterPass<AliasPass> X("alias", "Alias pass");

} // namespace llvm