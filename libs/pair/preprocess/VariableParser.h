#pragma once
#include "Common.h"

#include "DataParser.h"
#include "ValidParser.h"
#include "VarFinalizerParser.h"
#include "parser_util/VarNameParser.h"

namespace llvm {

class VariableParser {

public:
  VariableParser(Units& units) {
    // ordering matters!
    VarNameParser vnParser(units);
    ValidParser vParser(units);
    DataParser dParser(units);
    VarFinalizerParser vfParser(units);
  }
};

} // namespace llvm