#pragma once

#include "Functions.h"
#include "Locals.h"
#include "data_util/DbgInfo.h"

namespace llvm {

struct Globals {
  DbgInfo dbgInfo;
  Functions functions;
  Locals locals;
  AAResults& AAR;

  Globals(Module& M, AAResults& AAR_) : dbgInfo(M), AAR(AAR_) {}

  // globals methods----------------------------------

  void printFunctionNames(raw_ostream& O) const {
    dbgInfo.printFunctionNames(O);
  }

  auto& getAnalyzedFunctions() { return functions.getAnalyzedFunctions(); }

  auto& getAliasAnalysisResults() { return AAR; }

  void printFunctions(raw_ostream& O) const {
    functions.print(O);
    dbgInfo.print(O);
  }

  void createAliasGroups() { locals.createAliasGroups(&AAR); }

  // locals methods----------------------------------

  void setActiveFunction(Function* f) { locals.setFunction(f); }

  auto& getVariables() { return locals.getVariables(); }

  bool isIpInstruction(Instruction* i) const {
    return locals.isIpInstruction(i);
  }

  void printLocals(raw_ostream& O) const { locals.print(O); }
};

} // namespace llvm