#pragma once
#include "../data_util/DbgInfo.h"
#include "Common.h"
#include "DfUtil.h"

namespace llvm {

template <typename State> class DataflowResults {
public:
  // df types
  using AbstractState = State;
  using FunctionResults = std::map<Value*, AbstractState>;
  using ContextResults = std::map<Context, FunctionResults>;

private:
  Function* topFunction;
  ContextResults allResults;

public:
  void setFunction(Function* f) { topFunction = f; }

  auto& getDataflowResults() { return allResults; }

  auto& getFunctionResults(const Context& context) {
    return allResults[context];
  }

  bool inAllResults(const Context& context) {
    return allResults.count(context);
  }

  auto& getFinalState() {
    assert(topFunction);
    auto context = Context();
    auto& functionResults = allResults[context];
    auto* exitKey = Traversal::getFunctionExitKey(topFunction);
    auto& state = functionResults[exitKey];
    return state;
  }

  void clear() {
    topFunction = nullptr;
    for (auto& [context, functionResults] : allResults) {
      for (auto& [location, state] : functionResults) {
        state.clear();
      }
      functionResults.clear();
    }
    allResults.clear();
  }

  auto* getCurrentFunction(Context& context){
    assert(topFunction);
    if(auto* ci = context.getCallee())
      return ci->getCalledFunction();
    else
      return topFunction;
  }

  auto* getCallerFunction(Context& context){
    assert(topFunction);
    if(context.isTop())
      return (Function*)nullptr;
    if(auto* ci = context.getCaller())
      return ci->getCalledFunction();
    else
      return topFunction;
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