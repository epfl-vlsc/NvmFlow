#pragma once
#include "Common.h"

#include "DataParser.h"
#include "ValidParser.h"
#include "VarFiller.h"

namespace llvm {

class VariableParser {

public:
  VariableParser(Globals& globals) {
    // ordering matters!
    ValidParser vParser(globals);
    DataParser dParser(globals);
    VarFiller vFiller(globals);
  }
};

} // namespace llvm