#pragma once
#include "Common.h"

namespace llvm {

template <typename Globals, typename FunctionParser, typename VariableParser>
class Parser {
public:
  Parser(Module& M, Globals& globals) {
    // ordering matters!
    FunctionParser fParser(M, globals);
    fParser.parse();

#ifdef DBGMODE
    globals.printFunctions(errs());
#endif

    VariableParser vParser(globals);
  }
};

} // namespace llvm