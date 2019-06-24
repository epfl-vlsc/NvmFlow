#pragma once

#include "Common.h"
#include "ds/LogUnits.h"
#include "preprocess/LogParser.h"

namespace llvm {

class LogAnalyzer {
  Module& M;
  LogUnits units;

public:
  LogAnalyzer(Module& M_) : M(M_) {}

  void dataflow() {}

  void parse() {
    LogParser parser(M, units);
    units.print(llvm::errs());
  }

  void report() {}
};

} // namespace llvm