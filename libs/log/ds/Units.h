#pragma once

#include "Functions.h"
#include "Variables.h"

namespace llvm {

class Units : public Functions, public Variables {

public:
  void print(raw_ostream& O) const {
    printFunctions(O);
    O << "\n";
    printVariables(O);
    O << "\n";
  }
};

} // namespace llvm