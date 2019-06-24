

#include "ExpPass.h"
#include "Common.h"
#include "ExpAnalyzer.h"

namespace llvm {

void ExpPass::print(raw_ostream& OS, const Module* M) const { OS << "lol\n"; }

bool ExpPass::runOnModule(Module& m) {
  auto f = m.getFunction("_Z3setP1A");
  if(f){
  auto& aa = getAnalysis<AAResultsWrapperPass>(*f).getAAResults();
  traverse(*f, aa);
  }
  return false;
}

void ExpPass::getAnalysisUsage(AnalysisUsage& AU) const {
  AU.addRequired<AAResultsWrapperPass>();
  AU.setPreservesAll();
}

char ExpPass::ID = 0;
RegisterPass<ExpPass> X("exp", "Experimental pass");

} // namespace llvm