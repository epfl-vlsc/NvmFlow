#pragma once

#include "LogFunctions.h"
#include "LogVariables.h"

namespace llvm {

class LogUnits : public LogFunctions, public LogVariables {

public:
  void print(raw_ostream& O) const {
    printFunctions(O);
    O << "\n";
  }
};

} // namespace llvm