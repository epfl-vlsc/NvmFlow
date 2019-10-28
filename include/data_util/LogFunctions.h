#pragma once
#include "Common.h"

#include "data_util/AnnotatedFunctions.h"
#include "data_util/FunctionsBase.h"
#include "data_util/NamedFunctions.h"

namespace llvm {

class LogFunctions : public FunctionsBase {
private:
  static constexpr const char* TX = "TxCode";

  LoggingFunctions loggingFunctions;
  TxBeginFunctions txBeginFunctions;
  TxEndFunctions txEndFunctions;

public:
  LogFunctions() : FunctionsBase(TX) {}

  bool skipFunction(Function* f) const override {
    return isLoggingFunction(f) || isTxBeginFunction(f) || isTxEndFunction(f) ||
           isSkippedFunction(f);
  }

  bool isLoggingFunction(Function* f) const {
    return loggingFunctions.count(f);
  }

  bool isTxBeginFunction(Function* f) const {
    return txBeginFunctions.count(f);
  }

  bool isTxEndFunction(Function* f) const { return txEndFunctions.count(f); }

  void addAnnotFuncChecker(Function* f, StringRef annotation) override {
    loggingFunctions.addAnnotFunc(f, annotation);
    txBeginFunctions.addAnnotFunc(f, annotation);
    txEndFunctions.addAnnotFunc(f, annotation);
  }

  void addNamedFuncChecker(Function* f, StringRef name) override {
    loggingFunctions.addNamedFunc(f, name);
    txBeginFunctions.addNamedFunc(f, name);
    txEndFunctions.addNamedFunc(f, name);
  }

  void printChecker(raw_ostream& O) const override {
    loggingFunctions.print(O);
    txBeginFunctions.print(O);
    txEndFunctions.print(O);
  }
};

} // namespace llvm