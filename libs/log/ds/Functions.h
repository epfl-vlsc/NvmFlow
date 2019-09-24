#pragma once
#include "Common.h"

#include "data_util/AnnotatedFunctions.h"
#include "data_util/NamedFunctions.h"

namespace llvm {

class Functions {
public:
  static constexpr const char* NVM = "NvmCode";
  static constexpr const char* SKIP = "SkipCode";

private:
  AnnotatedFunctions analyzedFunctions;
  FunctionSet allAnalyzedFunctions;
  AnnotatedFunctions skippedFunctions;

  LoggingFunctions loggingFunctions;
  TxbeginFunctions txbeginFunctions;
  TxendFunctions txendFunctions;

public:
  Functions() : analyzedFunctions(NVM), skippedFunctions(SKIP) {}

  auto& getAnalyzedFunctions() { return analyzedFunctions; }

  void getUnitFunctions(Function* f, std::set<Function*>& visited) {
    visited.insert(f);

    for (auto& I : instructions(*f)) {
      if (auto* ci = dyn_cast<CallInst>(&I)) {
        auto* callee = ci->getCalledFunction();
        bool doIp = !visited.count(callee) && !skipFunction(callee);
        if (doIp) {
          getUnitFunctions(callee, visited);
        }
      }
    }
  }

  auto getUnitFunctions(Function* f) {
    std::set<Function*> visited;
    getUnitFunctions(f, visited);
    return visited;
  }

  auto getUnitFunctionSet(Function* f) {
    std::set<Function*> visited;
    getUnitFunctions(f, visited);

    FunctionSet funcSet(visited);
    return funcSet;
  }

  bool skipFunction(Function* f) const {
    return isLoggingFunction(f) || isTxbeginFunction(f) || isTxendFunction(f) ||
           isSkippedFunction(f);
  }

  bool isAnalyzedFunction(Function* f) const {
    return analyzedFunctions.count(f);
  }

  bool isSkippedFunction(Function* f) const {
    return !f || f->isIntrinsic() || skippedFunctions.count(f);
  }

  bool isLoggingFunction(Function* f) const {
    return loggingFunctions.count(f);
  }

  bool isTxbeginFunction(Function* f) const {
    return txbeginFunctions.count(f);
  }

  bool isTxendFunction(Function* f) const { return txendFunctions.count(f); }

  void insertAnnotatedFunction(Function* f, StringRef annotation) {
    analyzedFunctions.insertAnnotatedFunction(f, annotation);
    skippedFunctions.insertAnnotatedFunction(f, annotation);
    loggingFunctions.insertNamedFunction(f, realName);
    txbeginFunctions.insertNamedFunction(f, realName);
    txendFunctions.insertNamedFunction(f, realName);
  }

  void insertNamedFunction(Function* f) {
    auto name = f->getName();
    loggingFunctions.insertNamedFunction(f, name);
    txbeginFunctions.insertNamedFunction(f, name);
    txendFunctions.insertNamedFunction(f, name);
  }

  void insertToAllAnalyzed(Function* f) { allAnalyzedFunctions.insert(f); }

  void insertSkipFunction(Function* f) { skippedFunctions.insert(f); }

  void print(raw_ostream& O) const {
    O << "Functions Info\n";
    O << "--------------\n";

    analyzedFunctions.print(O);
    skippedFunctions.print(O);
    loggingFunctions.print(O);
    txbeginFunctions.print(O);
    txendFunctions.print(O);
    O << "\n";
  }
};

} // namespace llvm