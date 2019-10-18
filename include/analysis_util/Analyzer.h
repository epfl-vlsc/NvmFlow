#pragma once

#include "Common.h"
#include "GlobalStore.h"
#include "StateMachine.h"

namespace llvm {

template <typename Globals, typename Parser, typename State,
          typename Transition, typename BReporter>
class Analyzer {
  using FlowAnalyzer = StateMachine<Globals, State, Transition, BReporter>;

  Module& M;
  Globals globals;

public:
  Analyzer(Module& M_, AAResults& AAR_) : M(M_), globals(M_, AAR_) {
    errs() << "Parse\n";
    errs() << "-----\n";
    parse();

    errs() << "Dataflow Analysis\n";
    errs() << "-----------------\n";
    //dataflow();
  }

  void dataflow() {
    for (auto* f : globals.getAnalyzedFunctions()) {
      globals.setActiveFunction(f);

#ifdef DBGMODE
      globals.printLocals(errs());
#endif

#ifdef DATAFLOW
      FlowAnalyzer(M, globals).analyze(f);
#endif
    }
  }

  void parse() {
    Parser parser(M, globals);

#ifdef DBGMODE
    globals.printDbgInfo(errs());
#endif
  }
};

} // namespace llvm