#pragma once

#include "Common.h"
#include "ds/Units.h"
#include "global_util/DbgInfo.h"
#include "preprocess/Parser.h"

namespace llvm {

class Analyzer {
  Module& M;
  DbgInfo dbgInfo;
  Units units;

public:
  Analyzer(Module& M_) : M(M_), dbgInfo(M) {
    dbgInfo.print(llvm::errs());
    parse();
  }

  void dataflow() {}

  void parse() {
    Parser parser(M, dbgInfo, units);
    units.print(llvm::errs());
  }

  void report() {}
};

} // namespace llvm