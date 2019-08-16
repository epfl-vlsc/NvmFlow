#include "ConsPass.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

namespace llvm {

void ConsPass::print(raw_ostream& OS, const Module* m) const {
  OS << "print bugs here\n";
}

bool ConsPass::runOnModule(Module& M) {
  SetConstant useNvm(M);

  return true;
}

void ConsPass::getAnalysisUsage(AnalysisUsage& AU) const {}

char ConsPass::ID = 0;
RegisterPass<ConsPass> X("cons", "Cons pass");

} // namespace llvm