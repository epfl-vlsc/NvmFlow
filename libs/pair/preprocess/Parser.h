#pragma once
#include "Common.h"

#include "FunctionParser.h"
#include "VariableParser.h"

namespace llvm {

class Parser {

public:
  Parser(Module& M, Units& units) {
    // ordering matters!
    FunctionParser fParser(M, units);
    VariableParser vParser(M, units);
  }
};

} // namespace llvm