#pragma once
#include "Common.h"
#include "data_util/DbgInfo.h"

namespace llvm {

struct DbgPass : public ModulePass {
  static char ID;

  DbgPass() : ModulePass(ID) {}

  bool runOnModule(Module& M) override;

  void print(raw_ostream &OS, const Module *m) const override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

} // namespace llvm