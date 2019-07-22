#include "PairPass.h"

namespace llvm {

void PairPass::print(raw_ostream& OS, const Module* m) const {
  OS << "print bugs here\n";
}

bool PairPass::runOnModule(Module& M) {
  Analyzer analyzer(M);
  return false;
}

void PairPass::getAnalysisUsage(AnalysisUsage& AU) const {
}

char PairPass::ID = 0;
RegisterPass<PairPass> X("pair", "Pair pass");

} // namespace llvm