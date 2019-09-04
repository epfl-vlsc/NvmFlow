#pragma once
#include "Common.h"

#include "../ds/Globals.h"
#include "DebugParser.h"
#include "FunctionParser.h"
#include "VariableParser.h"

namespace llvm {

class Parser {

public:
  Parser(Module& M, Globals& globals) {
    // ordering matters!
    FunctionParser fParser(M, globals);
#ifdef DBGMODE
    globals.printFunctions(errs());
#endif

    DebugParser dParser(globals);
#ifdef DBGMODE
    globals.printDbgInfo(errs());
#endif

    VariableParser vParser(globals);
  }
};

} // namespace llvm