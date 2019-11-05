#pragma once
#include "Common.h"

#include "VarParser.h"
#include "DbgParser.h"
#include "VarFiller.h"

namespace llvm {

template <typename Globals> class VariableParser {
public:
  VariableParser(Globals& globals) {
    // ordering matters!
    DbgParser bParser(globals);
    VarParser vParser(globals);
    VarFiller vFiller(globals);
  }
};

} // namespace llvm