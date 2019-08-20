#pragma once
#include "Common.h"
#include "AliasParser.h"
//#include "DataParser.h"
#include "parser_util/VarNameParser.h"

namespace llvm {

class VariableParser {

public:
  VariableParser(Units& units) {
    // ordering matters!
    VarNameParser vnParser(units);
    AliasParser aParser(units);
    //DataParser dParser(units);
  }
};

} // namespace llvm