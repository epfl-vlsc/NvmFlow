#pragma once

#include "Common.h"
#include "ProgramStore.h"
#include "StateMachine.h"
#include "parser_util/Parser.h"

namespace llvm {

template <typename Globals, typename VarParser, typename State,
          typename Transition, typename BReporter>
class Analyzer {
  using FullParser = Parser<Globals, VarParser>;
  using FlowAnalyzer = StateMachine<Globals, State, Transition, BReporter>;

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

      FlowAnalyzer(M, globals).analyze(f);
    }
  }

  void parse() {
    FullParser parser(M, globals);

#ifdef DBGMODE
    globals.printDbgInfo(errs());
#endif
  }
};

} // namespace llvm