#pragma once

#include "Functions.h"
#include "Locals.h"
#include "data_util/DbgInfo.h"

namespace llvm {

struct Globals {
  DbgInfo dbgInfo;
  Functions functions;
  Locals locals;

  Globals(Module& M, AAResults& AAR) : dbgInfo(M), locals(AAR) {}

  // globals methods----------------------------------

  void printFunctionNames(raw_ostream& O) const {
    dbgInfo.printFunctionNames(O);
  }

  auto& getAnalyzedFunctions() { return functions.getAnalyzedFunctions(); }

  void printFunctions(raw_ostream& O) const { functions.print(O); }

  void printDbgInfo(raw_ostream& O) const { dbgInfo.print(O); }

  // locals methods----------------------------------

  void setActiveFunction(Function* f) { locals.setFunction(f); }

  auto& getVariables() { return locals.getVariables(); }

  bool isIpInstruction(Instruction* i) const {
    return locals.isIpInstruction(i);
  }

  void printLocals(raw_ostream& O) const { locals.print(O); }
};

} // namespace llvm