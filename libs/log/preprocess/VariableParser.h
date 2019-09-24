#pragma once
#include "Common.h"

#include "DbgParser.h"

namespace llvm {

template <typename Globals> class VariableParser {
public:
  VariableParser(Globals& globals) {
    // ordering matters!
    DbgParser dParser(globals);    
  }
};

} // namespace llvm