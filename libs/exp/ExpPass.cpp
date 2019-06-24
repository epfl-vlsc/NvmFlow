

#include "Common.h"
#include "ExpPass.h"

namespace llvm {

char ExpPass::ID = 0;

RegisterPass<ExpPass> X("exp", "Experimental pass");

// For an analysis pass, runOnModule should perform the actual analysis and
// compute the results. The actual output, however, is produced separately.
bool ExpPass::runOnModule(Module& m) {
  for (auto& f : m) {
    for (auto& bb : f) {
      for (auto& i : bb) {
        llvm::errs() << i << "\n";
      }
    }
  }

  return false;
}

void ExpPass::print(raw_ostream &OS, const Module *M) const{
    OS << "lol\n";
}

} // namespace llvm