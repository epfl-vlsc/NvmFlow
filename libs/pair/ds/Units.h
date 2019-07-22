#pragma once

#include "Functions.h"
#include "Variables.h"
#include "data_util/DbgInfo.h"

namespace llvm {

class Units {
  std::map<Function*, FunctionVariables> funcVars;

public:
  DbgInfo dbgInfo;
  Functions functions;
  FunctionVariables* activeFunction;

  Units(Module& M) : dbgInfo(M) {}

  void initDbgInfo() { dbgInfo.initDbgInfo(); }

  void setActiveFunction(Function* function) {
    activeFunction = &funcVars[function];
    activeFunction->setFunction(function);
  }

  auto& getAnalyzedFunctions() { return functions.getAnalyzedFunctions(); }

  auto& getVariables() { return activeFunction->getVariables(); }

  void printFunctions(raw_ostream& O) const {
    functions.print(O);
  }

  void printDbgInfo(raw_ostream& O) const {
    dbgInfo.print(O);
  }

  void printActiveFunction(raw_ostream& O) const {
    O << "*********************************\n";
    activeFunction->print(O);
    O << "---------------------------------\n";
  }
};

} // namespace llvm