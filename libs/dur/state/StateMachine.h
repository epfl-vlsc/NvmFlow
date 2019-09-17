#pragma once
#include "Common.h"

#include "BugReporter.h"
#include "FlowTypes.h"
#include "Lattice.h"
#include "Transfer.h"
#include "analysis_util/DataflowAnalysis.h"
#include "analysis_util/DataflowResults.h"
#include "ds/Globals.h"

namespace llvm {

class StateMachine {
public:
  using AbstractState = AbstractState;
  using AllResults = DataflowResults<AbstractState>;

private:
  Globals& globals;
  BugReporter breporter;
  Transfer transfer;
  AllResults allResults;

public:
  StateMachine(Module& M_, Globals& globals_)
      : globals(globals_), breporter(globals_, allResults),
        transfer(M_, globals_) {}

  void analyze(Function* function) {
    DataflowAnalysis dataflow(function, allResults, *this);

#ifdef DBGMODE
    allResults.print(errs());
#endif

    // breporter.checkBugs(function);

    allResults.clear();
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