#pragma once
#include "AliasParser.h"
#include "Common.h"
#include "DbgParser.h"

namespace llvm {

template <typename Globals> class VariableParser {
public:
  VariableParser(Globals& globals) {
    DbgParser dParser(globals);
    AliasParser aParser(globals);
  }
};

} // namespace llvm