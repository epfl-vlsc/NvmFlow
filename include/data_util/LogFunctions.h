#pragma once
#include "Common.h"

#include "data_util/AnnotatedFunctions.h"
#include "data_util/FunctionsBase.h"
#include "data_util/NamedFunctions.h"

namespace llvm {

class LogFunctions : public FunctionsBase {
private:
  static constexpr const char* TX = "TxCode";

  TxLogFunctions txLogFunctions;
  TxAllocFunctions txAllocFunctions;
  TxBeginFunctions txBeginFunctions;
  TxEndFunctions txEndFunctions;

public:
  LogFunctions() : FunctionsBase(TX) {}

  bool skipFunction(Function* f) const override {
    return isTxLogFunction(f) || isTxAllocFunction(f) || isTxBeginFunction(f) ||
           isTxEndFunction(f) || isSkippedFunction(f);
  }

  bool isTxLogFunction(Function* f) const { return txLogFunctions.count(f); }

  bool isTxAllocFunction(Function* f) const {
    return txAllocFunctions.count(f);
  }

  bool isTxBeginFunction(Function* f) const {
    return txBeginFunctions.count(f);
  }

  bool isTxEndFunction(Function* f) const { return txEndFunctions.count(f); }

  void addAnnotFuncChecker(Function* f, StringRef annotation) override {
    txLogFunctions.addAnnotFunc(f, annotation);
    txAllocFunctions.addAnnotFunc(f, annotation);
    txBeginFunctions.addAnnotFunc(f, annotation);
    txEndFunctions.addAnnotFunc(f, annotation);
  }

  void addNamedFuncChecker(Function* f, StringRef name) override {
    txLogFunctions.addNamedFunc(f, name);
    txAllocFunctions.addNamedFunc(f, name);
    txBeginFunctions.addNamedFunc(f, name);
    txEndFunctions.addNamedFunc(f, name);
  }

  void printChecker(raw_ostream& O) const override {
    txLogFunctions.print(O);
    txAllocFunctions.print(O);
    txBeginFunctions.print(O);
    txEndFunctions.print(O);
  }
};

} // namespace llvm