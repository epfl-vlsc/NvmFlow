#pragma once

#include "Functions.h"
#include "Variables.h"

namespace llvm {

struct Units {
  DbgInfo dbgInfo;
  Functions functions;
  Variables variables;
  AAResults& AAR;

  Units(Module& M, AAResults& AAR_) : dbgInfo(M), AAR(AAR_) {}

  auto& getAnalyzedFunctions() { return functions.getAnalyzedFunctions(); }

  auto& getAliasAnalysisResults() { return AAR; }

  void printFunctions(raw_ostream& O) const { functions.print(O); }

  void printDbgInfo(raw_ostream& O) const { dbgInfo.print(O); }

  void setActiveFunction(Function* function) {
    variables.setFunction(function);
  }

  void createAliasGroups(){
    variables.createAliasGroups(&AAR);
  }

  void printVariables(raw_ostream& O) const { variables.print(O); }

  auto& getVariables() { return variables.getVariables(); }

  bool isIpInstruction(Instruction* i) const {
    return variables.isIpInstruction(i);
  }
};

} // namespace llvm