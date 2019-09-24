#pragma once
#include "Common.h"

#include "DbgParser.h"
#include "VarParser.h"

namespace llvm {

template <typename Globals> class VariableParser {
public:
  VariableParser(Globals& globals) {
    // ordering matters!
    DbgParser dParser(globals);
    VarParser vParser(globals);
  }
};

} // namespace llvm