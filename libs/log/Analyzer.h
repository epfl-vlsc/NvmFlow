#pragma once

#include "Common.h"
#include "data_util/DbgInfo.h"
#include "ds/Units.h"
#include "preprocess/Parser.h"

namespace llvm {

class Analyzer {
  Module& M;
  ModulePass* pass;
  DbgInfo dbgInfo;
  Units units;

public:
  Analyzer(Module& M_, ModulePass* pass_) : M(M_), pass(pass_), dbgInfo(M) {
    dbgInfo.print(llvm::errs());
    parse();
  }

  void dataflow() {}

  void parse() {
    Parser parser(M, pass, dbgInfo, units);
    units.print(llvm::errs());
  }

  void report() {}
};

} // namespace llvm