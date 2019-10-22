#pragma once
#include "Common.h"
#include "PointsTo.h"

namespace llvm {

struct AliasPass : public ModulePass {
  static char ID;

  AliasPass() : ModulePass(ID) {}

  bool runOnModule(Module& M) override;

  void print(raw_ostream& OS, const Module* m) const override;

  void getAnalysisUsage(AnalysisUsage& AU) const override;
};

} // namespace llvm