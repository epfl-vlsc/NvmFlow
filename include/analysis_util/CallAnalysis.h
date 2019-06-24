#pragma once

#include "llvm/Analysis/CallGraph.h"

#include "Common.h"

namespace llvm{
/*
      CallGraphWrapperPass analysis;
  analysis.runOnModule(*M);
  CallGraph& CG = analysis.getCallGraph();

  for (auto& [_, cgn] : CG) {
    auto* function = cgn->getFunction();
    if (function)
      llvm::errs() << function->getName() << "\n";

    for (auto& [_, cgn2] : *cgn) {
      auto* function = cgn2->getFunction();
      if (function)
        llvm::errs() << "\t" << function->getName() << "\n";
    }
  }
  */

void traverse(Function& F) {
  for (auto& BB : F) {
    for (auto& I : BB) {
      llvm::errs() << I << "\n";
      if (I.getType()->isPointerTy()) {
        llvm::errs() << I << "\n";
      } else if (auto CI = dyn_cast<CallInst>(&I)) {
        auto& F2 = *CI->getCalledFunction();
        traverse(F2);
      }
    }
  }
}


}