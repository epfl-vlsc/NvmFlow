#pragma once
#include "Common.h"

#include "global_util/VariableSet.h"

namespace llvm {

class Variables {
protected:
  VariableSet analyzedVariables;

public:
  void printVariables(raw_ostream& O) const { analyzedVariables.print(O); }

  bool isAnalyzedVariable(VariableInfo* v) {
    return analyzedVariables.count(v);
  }
};

} // namespace llvm