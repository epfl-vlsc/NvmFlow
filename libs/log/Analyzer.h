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
    units.print(llvm::errs());

    dataflow();
  }

  void dataflow() {
    StateMachine stateMachine(units);

    for (auto* function : stateMachine.getAnalyzedFunctions()) {
      stateMachine.analyze(function);
    }
  }

  void parse() { Parser parser(M, units); }

  void report() {}
};

} // namespace llvm