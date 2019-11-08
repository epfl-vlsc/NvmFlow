#pragma once
#include "Common.h"

#include "AnnotatedFunctions.h"
#include "NamedFunctions.h"

namespace llvm {

class FunctionsBase {
  using FunctionMap = std::map<Function*, FunctionSet>;

public:
  static constexpr const char* NVM = "NvmCode";
  static constexpr const char* SKIP = "SkipCode";

protected:
  AnnotatedFunctions analyzedFunctions;
  FunctionMap allAnalyzedFunctions;
  AnnotatedFunctions skippedFunctions;
  StoreFunctions storeFunctions;

public:
  FunctionsBase() : analyzedFunctions(NVM), skippedFunctions(SKIP) {}

  FunctionsBase(const char* TX)
      : analyzedFunctions(TX), skippedFunctions(SKIP) {}

  auto& getAnalyzedFunctions() { return analyzedFunctions; }

  auto& getAllAnalyzedFunctions() { return allAnalyzedFunctions; }

  auto getUnitFunctions(Function* f) {
    assert(allAnalyzedFunctions.count(f));
    return allAnalyzedFunctions[f];
  }

  bool funcCallsFunc(Function* from, Function* to) {
    if (allAnalyzedFunctions.count(from)) {
      auto& callees = allAnalyzedFunctions[from];
      return callees.count(to);
    }
    return false;
  }

  bool isAnalyzedFunction(Function* f) const {
    return analyzedFunctions.count(f);
  }

  bool isSkippedFunction(Function* f) const {
    return !f || f->isIntrinsic() || skippedFunctions.count(f);
  }

  bool isStoreFunction(Function* f) const { return storeFunctions.count(f); }

  void addAnnotFunc(Function* f, StringRef annotation) {
    analyzedFunctions.addAnnotFunc(f, annotation);
    skippedFunctions.addAnnotFunc(f, annotation);
    storeFunctions.addAnnotFunc(f, annotation);
    addAnnotFuncChecker(f, annotation);
  }

  void addNamedFunc(Function* f) {
    auto name = f->getName();
    storeFunctions.addNamedFunc(f, name);
    addNamedFuncChecker(f, name);
  }

  void insertSkipFunction(Function* f) { skippedFunctions.insert(f); }

  virtual void addAnnotFuncChecker(Function* f, StringRef annotation) = 0;

  virtual void addNamedFuncChecker(Function* f, StringRef name) = 0;

  virtual void printChecker(raw_ostream& O) const = 0;

  virtual bool skipFunction(Function* f) const = 0;

  void print(raw_ostream& O) const {
    O << "Functions Info\n";
    O << "--------------\n";

    analyzedFunctions.print(O);
    skippedFunctions.print(O);
    storeFunctions.print(O);
    printChecker(O);

    O << "\n";
  }

  void printAllAnalyzed(raw_ostream& O) {
    O << "Functions Info\n";
    O << "--------------\n";

    for (auto& [f, fs] : allAnalyzedFunctions) {
      O << f->getName() << ":";
      for (auto* callee : fs) {
        O << callee->getName() << ",";
      }
      O << "\n";
    }

    O << "\n";
  }
};

} // namespace llvm