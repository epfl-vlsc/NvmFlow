#pragma once
#include "Common.h"

namespace llvm {

struct ExpPass : public ModulePass {
  static char ID;

  ExpPass() : ModulePass(ID) {}

  bool runOnModule(Module& M) override;

  void print(raw_ostream &OS, const Module *m) const override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

} // namespace llvm