#pragma once

#include "data_util/DbgInfo.h"

namespace llvm {

template<typename Functions, typename Locals>
struct ProgramStore {
  DbgInfo dbgInfo;
  Functions functions;
  Locals locals;

  ProgramStore(Module& M, AAResults& AAR) : dbgInfo(M), locals(AAR) {}

  // globals methods----------------------------------

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