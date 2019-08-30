#pragma once

#include "Functions.h"
#include "data_util/DbgInfo.h"
#include "Locals.h"

namespace llvm {

struct Globals {
  DbgInfo dbgInfo;
  Functions functions;
  Locals locals;

  Globals(Module& M) : dbgInfo(M) {}

  // globals methods----------------------------------

  void printFunctionNames(raw_ostream& O) const {
    dbgInfo.printFunctionNames(O);
  }

  auto& getAnalyzedFunctions() { return functions.getAnalyzedFunctions(); }

  void printFunctions(raw_ostream& O) const {
    functions.print(O);
    dbgInfo.print(O);
  }

  // locals methods----------------------------------

  void setActiveFunction(Function* f) {
    locals.setFunction(f);
  }

  auto& getVariables() { return locals.getVariables(); }

  bool isIpInstruction(Instruction* i) const {
    return locals.isIpInstruction(i);
  }

  void printLocals(raw_ostream& O) const { locals.print(O);}
};

} // namespace llvm