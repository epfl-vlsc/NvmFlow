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

  PfenceFunctions pfenceFunctions;
  VfenceFunctions vfenceFunctions;
  FlushFunctions flushFunctions;
  FlushFenceFunctions flushFenceFunctions;

public:
  Functions() : analyzedFunctions(NVM), skippedFunctions(SKIP) {}

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

  bool skipFunction(Function* f) const {
    return isPfenceFunction(f) || isVfenceFunction(f) || isFlushFunction(f) ||
           isFlushFenceFunction(f) || isSkippedFunction(f);
  }

  bool isAnalyzedFunction(Function* f) const {
    return analyzedFunctions.count(f);
  }

  bool isSkippedFunction(Function* f) const {
    return !f || f->isIntrinsic() || skippedFunctions.count(f);
  }

  bool isPfenceFunction(Function* f) const { return pfenceFunctions.count(f); }

  bool isVfenceFunction(Function* f) const { return vfenceFunctions.count(f); }

  bool isFlushFunction(Function* f) const { return flushFunctions.count(f); }

  bool isFlushFenceFunction(Function* f) const {
    return flushFenceFunctions.count(f);
  }

  void insertAnnotatedFunction(Function* f, StringRef annotation) {
    analyzedFunctions.insertAnnotatedFunction(f, annotation);
    skippedFunctions.insertAnnotatedFunction(f, annotation);
    pfenceFunctions.insertNamedFunction(f, annotation);
    vfenceFunctions.insertNamedFunction(f, annotation);
    flushFunctions.insertNamedFunction(f, annotation);
    flushFenceFunctions.insertNamedFunction(f, annotation);
  }

  void insertNamedFunction(Function* f, StringRef realName) {
    pfenceFunctions.insertNamedFunction(f, realName);
    vfenceFunctions.insertNamedFunction(f, realName);
    flushFunctions.insertNamedFunction(f, realName);
    flushFenceFunctions.insertNamedFunction(f, realName);
  }

  void insertToAllAnalyzed(Function* f) { allAnalyzedFunctions.insert(f); }

  void print(raw_ostream& O) const {
    O << "Functions Info\n";
    O << "--------------\n";

    analyzedFunctions.print(O);
    skippedFunctions.print(O);
    pfenceFunctions.print(O);
    vfenceFunctions.print(O);
    flushFunctions.print(O);
    flushFenceFunctions.print(O);
    O << "\n";
  }
};

} // namespace llvm