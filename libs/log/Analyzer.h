#pragma once

#include "Common.h"

#include "data_util/DbgInfo.h"
#include "ds/Units.h"
#include "preprocess/Parser.h"

/*
#include "state/StateMachine.h"
#include "analysis_util/DataflowAnalysis.h"
 */

namespace llvm {

class Analyzer {
  Module& M;
  Units units;

public:
  Analyzer(Module& M_) : M(M_), units(M_) {
    parse();
    units.print(llvm::errs());

    dataflow();
  }

  void dataflow() {
    for(auto* function: units.getAnalyzedFunctions()){
      units.setActiveFunction(function);
      units.printActiveFunction(llvm::errs());
    }
    
    /*
    StateMachine stateMachine(M, units);

    //stateMachine.analyze();
     */
  }

  void parse() { Parser parser(M, units); }

  void report() {}
};

} // namespace llvm