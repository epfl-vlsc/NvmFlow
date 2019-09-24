#pragma once
#include "Common.h"

#include "checker_util/FunctionsBase.h"
#include "data_util/AnnotatedFunctions.h"
#include "data_util/NamedFunctions.h"

namespace llvm {

class Functions : public FunctionsBase {
private:
  LoggingFunctions loggingFunctions;
  TxBeginFunctions txBeginFunctions;
  TxEndFunctions txEndFunctions;

public:
  bool skipFunction(Function* f) const {
    return isLoggingFunction(f) || isTxbeginFunction(f) || isTxendFunction(f) ||
           isSkippedFunction(f);
  }

  bool isLoggingFunction(Function* f) const {
    return loggingFunctions.count(f);
  }

  bool isTxbeginFunction(Function* f) const {
    return txBeginFunctions.count(f);
  }

  bool isTxendFunction(Function* f) const { return txEndFunctions.count(f); }

  void insertAnnotatedFunction(Function* f, StringRef annotation) {
    analyzedFunctions.insertAnnotatedFunction(f, annotation);
    skippedFunctions.insertAnnotatedFunction(f, annotation);
    loggingFunctions.insertNamedFunction(f, annotation);
    txBeginFunctions.insertNamedFunction(f, annotation);
    txEndFunctions.insertNamedFunction(f, annotation);
  }

  void insertNamedFunction(Function* f) {
    auto name = f->getName();
    loggingFunctions.insertNamedFunction(f, name);
    txBeginFunctions.insertNamedFunction(f, name);
    txEndFunctions.insertNamedFunction(f, name);
  }

  void printChecker(raw_ostream& O) const {
    loggingFunctions.print(O);
    txBeginFunctions.print(O);
    txEndFunctions.print(O);
  }
};

} // namespace llvm