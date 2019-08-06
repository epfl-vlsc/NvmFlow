#pragma once

#include "Functions.h"
#include "Variables.h"

namespace llvm {

struct Units {
  DbgInfo dbgInfo;
  Functions functions;
  //Variables variables;

  Units(Module& M) : dbgInfo(M) {}

  auto& getAnalyzedFunctions() { return functions.getAnalyzedFunctions(); }

  void printFunctions(raw_ostream& O) const { functions.print(O); }

  void printDbgInfo(raw_ostream& O) const { dbgInfo.print(O); }
/*
  void setActiveFunction(Function* function) {
    variables.setFunction(function);
  }

  void printVariables(raw_ostream& O) const { variables.print(O); }

  auto& getVariables() { return variables.getVariables(); }

  bool isIpInstruction(Instruction* i) const {
    return variables.isIpInstruction(i);
  }
 */
};

} // namespace llvm