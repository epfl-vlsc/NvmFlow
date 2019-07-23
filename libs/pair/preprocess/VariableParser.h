#pragma once
#include "Common.h"

#include "ValidParser.h"

namespace llvm {

class VariableParser {

public:
  VariableParser(Module& M, Units& units) {
    // ordering matters!
    ValidParser vParser(M, units);
    //DataParser dParser(M, units);
  }
};

} // namespace llvm