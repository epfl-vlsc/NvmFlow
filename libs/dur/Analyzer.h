#pragma once

#include "Common.h"

#include "ds/Globals.h"
#include "preprocess/Parser.h"
#include "state/StateMachine.h"

namespace llvm {

class Analyzer {
  Module& M;
  Globals globals;

public:
  Analyzer(Module& M_, AAResults& AAR_) : M(M_), globals(M_, AAR_) {
    parse();

    dataflow();
  }

  void dataflow() {
    errs() << "Dataflow Analysis\n";
    errs() << "-----------------\n";

    for (auto* f : globals.getAnalyzedFunctions()) {
      globals.setActiveFunction(f);

#ifdef DBGMODE
      globals.printLocals(errs());
#endif

      StateMachine(M, globals).analyze(f);
    }
  }

  void parse() {
#ifdef DBGMODE
    globals.printFunctionNames(errs());
#endif

    Parser parser(M, globals);

#ifdef DBGMODE
    globals.printFunctions(errs());
#endif
  }
};

} // namespace llvm