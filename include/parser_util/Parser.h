#pragma once
#include "Common.h"
#include "FunctionParser.h"

namespace llvm {

template <typename Globals, typename VariableParser> class Parser {
public:
  Parser(Module& M, Globals& globals) {
    // ordering matters!
    FunctionParser<Globals> fParser(M, globals);
#ifdef DBGMODE
    globals.printFunctions(errs());
#endif

    // DebugParser dParser(globals);
#ifdef DBGMODE
    //globals.printDbgInfo(errs());
#endif

    // VariableParser vParser(globals);
  }
};

} // namespace llvm