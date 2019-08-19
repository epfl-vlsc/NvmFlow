#include "DurPass.h"

namespace llvm {

void DurPass::print(raw_ostream& OS, const Module* m) const {
  OS << "print bugs here\n";
}

bool DurPass::runOnModule(Module& M) {
  // initialize alias analysis
  auto& TLI = getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();
  AAResults AAR(TLI);

  //boost analysis with andersen analysis
  auto& aaResults = getAnalysis<CFLAndersAAWrapperPass>().getResult();
  AAR.addAAResult(aaResults);

  Analyzer analyzer(M, AAR);
  return false;
}

void DurPass::getAnalysisUsage(AnalysisUsage& AU) const {
  AU.addRequired<TargetLibraryInfoWrapperPass>();
  AU.addRequired<CFLAndersAAWrapperPass>();

  AU.setPreservesAll();
}

char DurPass::ID = 0;
RegisterPass<DurPass> X("dur", "Durable pointer pass");

} // namespace llvm