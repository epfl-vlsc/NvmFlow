#pragma once
#include "Common.h"

#include "AnnotatedFunctions.h"
#include "FunctionsBase.h"
#include "NamedFunctions.h"

namespace llvm {

class PersistFunctions : public FunctionsBase {
private:
  PfenceFunctions pfenceFunctions;
  VfenceFunctions vfenceFunctions;
  FlushFunctions flushFunctions;
  FlushFenceFunctions flushFenceFunctions;

public:
  bool skipFunction(Function* f) const override {
    return isPfenceFunction(f) || isVfenceFunction(f) || isFlushFunction(f) ||
           isFlushFenceFunction(f) || isSkippedFunction(f);
  }

  bool isPfenceFunction(Function* f) const { return pfenceFunctions.count(f); }

  bool isVfenceFunction(Function* f) const { return vfenceFunctions.count(f); }

  bool isFlushFunction(Function* f) const { return flushFunctions.count(f); }

  bool isFlushFenceFunction(Function* f) const {
    return flushFenceFunctions.count(f);
  }

  bool isAnyFlushFunction(Function* f) const {
    return flushFunctions.count(f) || flushFenceFunctions.count(f);
  }

  bool isAnyFlushFunction(Instruction* i) const {
    assert(i);
    if (auto* ci = dyn_cast<CallInst>(i)) {
      auto* f = ci->getCalledFunction();
      return isAnyFlushFunction(f);
    }

    return false;
  }

  void addAnnotFuncChecker(Function* f, StringRef annotation) override {
    pfenceFunctions.addAnnotFunc(f, annotation);
    vfenceFunctions.addAnnotFunc(f, annotation);
    flushFunctions.addAnnotFunc(f, annotation);
    flushFenceFunctions.addAnnotFunc(f, annotation);
  }

  void addNamedFuncChecker(Function* f, StringRef name) override {
    pfenceFunctions.addNamedFunc(f, name);
    vfenceFunctions.addNamedFunc(f, name);
    flushFunctions.addNamedFunc(f, name);
    flushFenceFunctions.addNamedFunc(f, name);
  }

  void printChecker(raw_ostream& O) const override {
    pfenceFunctions.print(O);
    vfenceFunctions.print(O);
    flushFunctions.print(O);
    flushFenceFunctions.print(O);
  }
};

} // namespace llvm