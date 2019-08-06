#pragma once
#include "Common.h"

#include "data_util/AnnotatedFunctions.h"
#include "data_util/NamedFunctions.h"

namespace llvm {

class Functions {
public:
  static constexpr const char* LOG = "LogCode";
  static constexpr const char* SKIP = "SkipCode";

private:
  AnnotatedFunctions analyzedFunctions;
  AnnotatedFunctions skippedFunctions;

  LoggingFunctions loggingFunctions;
  TxbeginFunctions txbeginFunctions;
  TxendFunctions txendFunctions;

  bool useTx;

public:
  Functions() : analyzedFunctions(LOG), skippedFunctions(SKIP), useTx(false) {}

  auto& getAnalyzedFunctions() { return analyzedFunctions; }

  void getUnitFunctions(Function* f, std::set<Function*>& visited) {
    visited.insert(f);

    for (auto& I : instructions(*f)) {
      if (auto* ci = dyn_cast<CallInst>(&I)) {
        auto* callee = ci->getCalledFunction();
        bool doIp = !callee->isDeclaration() && !visited.count(callee) &&
                    !skipFunction(callee);
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
    return isLoggingFunction(f) || isTxbeginFunction(f) || isTxendFunction(f) ||
           isSkippedFunction(f);
  }

  bool isAnalyzedFunction(Function* f) const {
    return analyzedFunctions.count(f);
  }

  bool isSkippedFunction(Function* f) const {
    return skippedFunctions.count(f);
  }

  bool isLoggingFunction(Function* f) const {
    return loggingFunctions.count(f);
  }

  bool isTxbeginFunction(Function* f) const {
    return txbeginFunctions.count(f);
  }

  bool isTxendFunction(Function* f) const { return txendFunctions.count(f); }

  void setTxMode() {
    useTx = !txbeginFunctions.empty() && !txendFunctions.empty();
    if (useTx) {
      assert(txbeginFunctions.size() == 1 && txendFunctions.size() == 1 &&
             "must have one type of tx");
    }
  }

  bool isUseTx() const { return useTx; }

  void insertAnnotatedFunction(Function* f, StringRef annotation) {
    analyzedFunctions.insertAnnotatedFunction(f, annotation);
    skippedFunctions.insertAnnotatedFunction(f, annotation);
  }

  void insertNamedFunction(Function* f, StringRef realName) {
    loggingFunctions.insertNamedFunction(f, realName);
    txbeginFunctions.insertNamedFunction(f, realName);
    txendFunctions.insertNamedFunction(f, realName);
  }

  void print(raw_ostream& O) const {
    O << "Functions Info\n";
    analyzedFunctions.print(O);
    skippedFunctions.print(O);
    loggingFunctions.print(O);
    if (useTx) {
      txbeginFunctions.print(O);
      txendFunctions.print(O);
    }
    O << "\n";
  }
};

} // namespace llvm