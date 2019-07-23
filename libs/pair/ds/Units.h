#pragma once

#include "Functions.h"
#include "Variables.h"

namespace llvm {

struct Units {
  DbgInfo dbgInfo;
  Functions functions;
  Variables variables;

  Units(Module& M) : dbgInfo(M) {}

  void setActiveFunction(Function* function) {
    variables.setFunction(function);
  }

  auto& getAnalyzedFunctions() { return functions.getAnalyzedFunctions(); }

  auto& getVariables() { return variables.getVariables(); }

  void printFunctions(raw_ostream& O) const { functions.print(O); }

  void printDbgInfo(raw_ostream& O) const { dbgInfo.print(O); }

  void printVariables(raw_ostream& O) const { variables.print(O); }

  bool isIpInstruction(Instruction* i) const {
    variables.isIpInstruction(i);
  }
};

} // namespace llvm