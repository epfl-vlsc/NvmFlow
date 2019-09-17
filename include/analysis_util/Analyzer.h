#pragma once

#include "Common.h"
#include "ProgramStore.h"
//#include "StateMachine.h"
#include "parser_util/Parser.h"

namespace llvm {

template <typename Globals, typename VarParser, typename State>
/*, typename Lattice, typename Transfer,
          typename BugReporter*/
class Analyzer {
  using FullParser = Parser<Globals, VarParser>;
  /*

  using Transition = typename Transfer<Globals, AbstractState>;
  using BReporter = typename BugReporter<Globals, AbstractState>;
  using SM = StateMachine<Globals, Variable, Lattice, Transition, BReporter>;
  */

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

      // StateMachine(M, globals).analyze(f);
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