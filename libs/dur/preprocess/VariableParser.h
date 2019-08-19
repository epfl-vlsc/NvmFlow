#pragma once
#include "Common.h"
#include "AliasParser.h"
//#include "DataParser.h"

namespace llvm {

class VariableParser {

public:
  VariableParser(Units& units) {
    // ordering matters!
    AliasParser aParser(units);
    //DataParser dParser(units);
  }
};

} // namespace llvm