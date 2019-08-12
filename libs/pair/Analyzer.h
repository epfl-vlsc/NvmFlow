#pragma once

#include "Common.h"

#include "data_util/DbgInfo.h"
#include "ds/Units.h"
#include "preprocess/Parser.h"
#include "state/StateMachine.h"

namespace llvm {

class Analyzer {
  Module& M;
  Units units;

public:
  Analyzer(Module& M_) : M(M_), units(M_) {
    parse();

    dataflow();
  }

  void dataflow() {
    StateMachine stateMachine(M, units);
    for (auto* function : stateMachine.getAnalyzedFunctions()) {
      stateMachine.analyze(function);
    }
  }

  void parse() {
    Parser parser(M, units);

#ifdef DBGMODE
    units.printDbgInfo(errs());
    units.printFunctions(errs());
#endif
  }

  void report() {}
};

} // namespace llvm