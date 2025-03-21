#pragma once

#include "data_util/DbgInfo.h"

namespace llvm {

template <typename Functions, typename Locals> struct GlobalStore {
  DbgInfo dbgInfo;
  Functions functions;
  Locals locals;
  AAResults& AAR;

  GlobalStore(Module& M, AAResults& AAR_) : dbgInfo(M), AAR(AAR_) {}

  // globals methods----------------------------------

  auto& getAnalyzedFunctions() { return functions.getAnalyzedFunctions(); }

  void printFunctions(raw_ostream& O) const { functions.print(O); }

  void printDbgInfo(raw_ostream& O) const { dbgInfo.print(O); }

  auto& getAliasAnalysis() { return AAR; }

  // locals methods----------------------------------

  void setActiveFunction(Function* f) { locals.setFunction(f); }

  auto& getVariables() { return locals.getVariables(); }

  bool isIpInstruction(Instruction* i) const {
    return locals.isIpInstruction(i);
  }

  void printLocals(raw_ostream& O) const { locals.print(O); }
};

} // namespace llvm