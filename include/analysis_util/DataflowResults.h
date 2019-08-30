#pragma once
#include "../data_util/DbgInfo.h"
#include "Common.h"
#include "DfUtil.h"

namespace llvm {

template <typename StateMachine> struct DataflowResults {
  // df results
  using AbstractState = typename StateMachine::AbstractState;
  using FunctionResults = std::map<Value*, AbstractState>;
  using ContextResults = std::map<Context, FunctionResults>;

  ContextResults allResults;

  auto& getDataflowResults() { return allResults; }

  auto& getFunctionResults(const Context& context) {
    return allResults[context];
  }

  void print(raw_ostream& O) const {
    O << "---------------------------------\n";
    O << "all results:\n";
    for (auto& [context, functionResults] : allResults) {
      O << context.getName() << "\n";
      for (auto& [location, state] : functionResults) {
        DbgInstr::printLocation(location, O);
        O << "\n";
        for (auto& [latVar, latVal] : state) {
          O << "\t" << latVar->getName() << " " << latVal.getName() << "\n";
        }
      }
    }
    O << "---------------------------------\n";
  }
};

} // namespace clang::ento::nvm