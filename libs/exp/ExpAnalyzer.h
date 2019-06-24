#pragma once

#include "Common.h"

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"

namespace llvm {

void traverse(Function& f, std::set<Value*>& ptrs) {
  for (auto& bb : f) {
    for (auto& i : bb) {
      if (i.getType()->isPointerTy()) {
        llvm::errs() << i << "\n";
        ptrs.insert(&i);
      } else if (auto ci = dyn_cast<CallInst>(&i)) {
        auto f2 = ci->getCalledFunction();
        if (f2) {
          traverse(*f2, ptrs);
        }
      }
    }
  }
}

void traverse(Function& f, AliasAnalysis& AA) {
  std::set<Value*> ptrs;

  traverse(f, ptrs);

  for (Value* P1 : ptrs) {
    for (Value* P2 : ptrs) {

      bool res =
          AA.alias(P1, LocationSize::unknown(), P2, LocationSize::unknown());
    
    llvm::errs() << (int)res << " " << *P1 << *P2 << "\n";
    }
  }
}

} // namespace llvm