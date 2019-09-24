#pragma once
#include "Common.h"

#include "AnnotatedFunctions.h"
#include "NamedFunctions.h"

namespace llvm {

class FunctionsBase {
public:
  static constexpr const char* NVM = "NvmCode";
  static constexpr const char* SKIP = "SkipCode";

protected:
  AnnotatedFunctions analyzedFunctions;
  FunctionSet allAnalyzedFunctions;
  AnnotatedFunctions skippedFunctions;

public:
  FunctionsBase() : analyzedFunctions(NVM), skippedFunctions(SKIP) {}

  auto& getAnalyzedFunctions() { return analyzedFunctions; }

  auto& getAllAnalyzedFunctions() { return allAnalyzedFunctions; }

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

  bool isAnalyzedFunction(Function* f) const {
    return analyzedFunctions.count(f);
  }

  bool isSkippedFunction(Function* f) const {
    return !f || f->isIntrinsic() || skippedFunctions.count(f);
  }

  virtual void insertAnnotatedFunction(Function* f, StringRef annotation) = 0;

  virtual void insertNamedFunction(Function* f) = 0;

  virtual void printChecker(raw_ostream& O) const = 0;

  virtual bool skipFunction(Function* f) const = 0;

  void insertToAllAnalyzed(Function* f) { allAnalyzedFunctions.insert(f); }

  void insertSkipFunction(Function* f) {
    skippedFunctions.insert(f);
    allAnalyzedFunctions.remove(f);
  }

  void print(raw_ostream& O) const {
    O << "Functions Info\n";
    O << "--------------\n";

    analyzedFunctions.print(O);
    skippedFunctions.print(O);
    printChecker(O);

    O << "\n";
  }
};

} // namespace llvm