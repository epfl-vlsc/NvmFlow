

#include "ExpPass.h"
#include "Common.h"
#include "llvm/Analysis/CFLAndersAliasAnalysis.h"
#include "llvm/Analysis/CFLSteensAliasAnalysis.h"
#include "llvm/Analysis/MemoryLocation.h"
#include "llvm/Analysis/MemorySSA.h"

namespace llvm {

template <typename AA> void traverse(Function& f, AA& aa) {

  for (auto& bb : f) {
    for (auto& i : bb) {
      auto* x = aa.getMemoryAccess(&i);
      if (!x) {
        continue;
      }

      x->print(llvm::errs());
      llvm::errs() << i << "\n";

      auto* d = x->getDefiningAccess();
      if (!d) {
        continue;
      }

      d->print(llvm::errs());
      llvm::errs() << "lol\n";

    }
  }
}

template <typename AA> void traverse(Module& M, AA& aa) {
  auto f = M.getFunction("_Z2m3P1A");
  traverse(*f, aa);
}

void ExpPass::print(raw_ostream& OS, const Module* m) const { OS << "lol\n"; }

bool ExpPass::runOnModule(Module& M) {
  // auto& aa = getAnalysis<CFLSteensAAWrapperPass>().getResult();
  // auto& aa = getAnalysis<CFLAndersAAWrapperPass>().getResult();

  auto f = M.getFunction("_Z2m3P1A");
  if (!f)
    return false;

  auto& aa = getAnalysis<MemorySSAWrapperPass>(*f).getMSSA();
  aa.print(llvm::errs());
  traverse(M, aa);

  return false;
}

void ExpPass::getAnalysisUsage(AnalysisUsage& AU) const {
  // AU.addRequired<CFLSteensAAWrapperPass>();
  // AU.addRequired<CFLAndersAAWrapperPass>();
  AU.addRequired<MemorySSAWrapperPass>();
  AU.setPreservesAll();
}

char ExpPass::ID = 0;
RegisterPass<ExpPass> X("exp", "Experimental pass");

} // namespace llvm