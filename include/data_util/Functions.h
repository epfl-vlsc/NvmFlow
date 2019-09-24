#pragma once
#include "Common.h"

#include "FunctionsBase.h"
#include "AnnotatedFunctions.h"
#include "NamedFunctions.h"

namespace llvm {

class Functions : public FunctionsBase {
private:
  PfenceFunctions pfenceFunctions;
  VfenceFunctions vfenceFunctions;
  FlushFunctions flushFunctions;
  FlushFenceFunctions flushFenceFunctions;

public:
  bool skipFunction(Function* f) const {
    return isPfenceFunction(f) || isVfenceFunction(f) || isFlushFunction(f) ||
           isFlushFenceFunction(f) || isSkippedFunction(f);
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

  void insertNamedFunction(Function* f) {
    auto name = f->getName();
    pfenceFunctions.insertNamedFunction(f, name);
    vfenceFunctions.insertNamedFunction(f, name);
    flushFunctions.insertNamedFunction(f, name);
    flushFenceFunctions.insertNamedFunction(f, name);
  }

  void printChecker(raw_ostream& O) const {
    pfenceFunctions.print(O);
    vfenceFunctions.print(O);
    flushFunctions.print(O);
    flushFenceFunctions.print(O);
  }
};

} // namespace llvm