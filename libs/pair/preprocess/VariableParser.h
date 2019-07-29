#pragma once
#include "Common.h"

#include "ValidParser.h"
#include "DataParser.h"
#include "VarFinalizerParser.h"

namespace llvm {

class VariableParser {

public:
  VariableParser(Units& units) {
    // ordering matters!
    ValidParser vParser(units);
    DataParser dParser(units);
    VarFinalizerParser vfParser(units);
  }
};

} // namespace llvm