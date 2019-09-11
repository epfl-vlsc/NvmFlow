#pragma once
#include "Common.h"

namespace llvm {

struct ParsePass : public ModulePass {
  static char ID;

  ParsePass() : ModulePass(ID) {}

  bool runOnModule(Module& M) override;

  void print(raw_ostream &OS, const Module *m) const override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

} // namespace llvm