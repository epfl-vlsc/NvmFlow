#pragma once
#include "Common.h"

#include "DataParser.h"
#include "DbgParser.h"
#include "ValidParser.h"
#include "VarFiller.h"

namespace llvm {

template <typename Globals> class VariableParser {
public:
  VariableParser(Globals& globals) {
    // ordering matters!

    DbgParser bParser(globals);
    ValidParser vParser(globals);
    DataParser dParser(globals);
    VarFiller vFiller(globals);
  }
};

} // namespace llvm