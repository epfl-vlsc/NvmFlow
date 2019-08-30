#pragma once
#include "Common.h"

namespace llvm {

struct SimpPass : public ModulePass {
  static char ID;

  SimpPass() : ModulePass(ID) {}

  bool runOnModule(Module& M) override;

  void print(raw_ostream &OS, const Module *m) const override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

} // namespace llvm