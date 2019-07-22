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
    //parse();

#ifdef DBGMODE
    units.print(llvm::errs());
#endif

    // dataflow();
  }

  void dataflow() {
    /*
    StateMachine stateMachine(M, units);

    // auto f = M.getFunction("_ZN3Log8correct2Ev");
    // stateMachine.analyze(f);

    for (auto* function : stateMachine.getAnalyzedFunctions()) {
      stateMachine.analyze(function);
    }
     */
  }

  void parse() { Parser parser(M, units); }

  void report() {}
};

} // namespace llvm