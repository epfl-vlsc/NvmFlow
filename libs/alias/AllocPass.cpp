#include "AllocPass.h"
#include "data_util/NameFilter.h"

using namespace std;
namespace llvm {

void AllocPass::print(raw_ostream& OS, const Module* m) const {
  OS << "pass\n";
}

bool AllocPass::runOnModule(Module& M) {
  for (auto& F : M) {
    if (NameFilter::isAllocFunction(F)) {
      auto attr = F.getAttribute(AttributeList::ReturnIndex, Attribute::NoAlias);
      F.addAttribute(AttributeList::ReturnIndex, Attribute::NoAlias);
      attr = F.getAttribute(AttributeList::ReturnIndex, Attribute::NoAlias);
    }
  }

  return true;
}

void AllocPass::getAnalysisUsage(AnalysisUsage& AU) const {
  AU.addRequired<TargetLibraryInfoWrapperPass>();
  AU.setPreservesAll();
}

char AllocPass::ID = 0;
RegisterPass<AllocPass> X("alloc", "Alloc pass");

} // namespace llvm