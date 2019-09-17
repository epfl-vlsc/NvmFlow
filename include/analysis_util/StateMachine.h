#pragma once
#include "Common.h"

#include "DataflowAnalysis.h"
#include "DataflowResults.h"

namespace llvm {

template <typename Globals, typename Variable, typename Lattice,
          typename Transition, typename BReporter>
class StateMachine {
public:
  
  using AllResults = DataflowResults<AbstractState>;

private:
  Globals& globals;
  Transition transition;
  BReporter breporter;
  AllResults allResults;

public:
  StateMachine(Module& M_, Globals& globals_)
      : globals(globals_), transition(M_, globals_),
        breporter(globals_, allResults) {}

  void analyze(Function* function) {
    DataflowAnalysis dataflow(function, allResults, *this);

#ifdef DBGMODE
    allResults.print(errs());
#endif

    // breporter.checkBugs(function);

    allResults.clear();
  }

  void initLatticeValues(AbstractState& state) {
    transition.initLatticeValues(state);
  }

  bool handleInstruction(Instruction* i, AbstractState& state) {
    return transition.handleInstruction(i, state);
  }

  bool isIpInstruction(Instruction* i) const {
    return globals.isIpInstruction(i);
  }

  auto& getAnalyzedFunctions() { return globals.getAnalyzedFunctions(); }
};

} // namespace llvm