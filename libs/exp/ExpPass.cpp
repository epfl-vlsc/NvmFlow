

#include "ExpPass.h"
#include "Common.h"
#include "llvm/Analysis/CFLSteensAliasAnalysis.h"
#include "llvm/Analysis/MemoryLocation.h"

namespace llvm {

void traverse(Function& f, std::set<StoreInst*>& ptrs) {
  for (auto& bb : f) {
    for (auto& i : bb) {
      // if (i.getType()->isPointerTy()) {
      if (auto si = dyn_cast<StoreInst>(&i)) {
        llvm::errs() << si << "\n";
        ptrs.insert(si);
      } else if (auto ci = dyn_cast<CallInst>(&i)) {
        auto f2 = ci->getCalledFunction();
        if (f2) {
          traverse(*f2, ptrs);
        }
      }
    }
  }
}

template<typename AA>
void traverse(Function& f, AA& aa) {
  std::set<StoreInst*> ptrs;

  traverse(f, ptrs);

  for (auto p1 : ptrs) {
    for (auto p2 : ptrs) {
      auto m1 = MemoryLocation::get(p1);
      auto m2 = MemoryLocation::get(p2);
      bool res = aa.alias(m1, m2);
      llvm::errs() << (int)res << " " << *p1 << " " << *p2 << "\n";
    }
  }
}

void ExpPass::print(raw_ostream& OS, const Module* m) const { OS << "lol\n"; }

bool ExpPass::runOnModule(Module& M) {

  auto f = M.getFunction("_Z3setP1A");
  if (!f)
    return false;

  auto& aa = getAnalysis<CFLSteensAAWrapperPass>().getResult();

  traverse(*f, aa);

  for (auto& x : M.aliases()) {
    llvm::errs() << x.getName() << "\n";
  }

  return false;
}

void ExpPass::getAnalysisUsage(AnalysisUsage& AU) const {
  AU.addRequired<CFLSteensAAWrapperPass>();
  AU.setPreservesAll();
}

char ExpPass::ID = 0;
RegisterPass<ExpPass> X("exp", "Experimental pass");

} // namespace llvm