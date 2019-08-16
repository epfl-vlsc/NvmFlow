#pragma once
#include "Common.h"
#include "data_util/SetConstant.h"

namespace llvm {

struct ConsPass : public ModulePass {
  static char ID;

  ConsPass() : ModulePass(ID) {}

  bool runOnModule(Module& M) override;

  void print(raw_ostream &OS, const Module *m) const override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

} // namespace llvm