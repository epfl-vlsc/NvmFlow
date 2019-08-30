#pragma once
#include "Common.h"

#include "BugReporter.h"
#include "FlowTypes.h"
#include "Lattice.h"
#include "Transfer.h"
#include "analysis_util/DataflowAnalysis.h"
#include "ds/Globals.h"

namespace llvm {

class StateMachine {
public:
  using AbstractState = AbstractState;

private:
  Globals& globals;
  BugReporter breporter;
  Transfer transfer;

public:
  StateMachine(Module& M_, Globals& globals_)
      : globals(globals_), breporter(globals_), transfer(M_, globals_, breporter) {}

  void analyze(Function* function) {
    breporter.initUnit(function);

    DataflowAnalysis dataflow(function, *this);

#ifdef DBGMODE
    dataflow.print(errs());
#endif

    breporter.print(errs());
  }

  void run(){
    
  }

  void initLatticeValues(AbstractState& state) {
    transfer.initLatticeValues(state);
  }

  bool handleInstruction(Instruction* i, AbstractState& state) {
    return transfer.handleInstruction(i, state);
  }

  bool isIpInstruction(Instruction* i) const {
    return globals.isIpInstruction(i);
  }

  auto& getAnalyzedFunctions() { return globals.getAnalyzedFunctions(); }
};

} // namespace llvm