#pragma once
#include "Common.h"

#include "FieldParser.h"
#include "ObjParser.h"
#include "VarFinalizerParser.h"

namespace llvm {

class VariableParser {

public:
  VariableParser(Units& units) {
    // ordering matters!
    FieldParser vParser(units);
    ObjParser oParser(units);
    VarFinalizerParser vfParser(units);
  }
};

} // namespace llvm