#pragma once
#include "Common.h"
#include "FlushParser.h"
#include "WriteParser.h"

namespace llvm {

class VariableParser {

public:
  VariableParser(Units& units) {
    // ordering matters!
    units.finalizeDbgInfo();
    FlushParser fParser(units);
    WriteParser wParser(units);
  }
};

} // namespace llvm