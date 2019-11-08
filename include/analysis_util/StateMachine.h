#pragma once
#include "Common.h"

#include "DataflowAnalysis.h"
#include "DataflowResults.h"

namespace llvm {

template <typename Globals, typename AbstractState, typename Transition,
          typename BReporter>
class StateMachine {
public:
  using CurrentSM = StateMachine<Globals, AbstractState, Transition, BReporter>;
  using DfResults = DataflowResults<AbstractState>;
  using DfAnalysis = DataflowAnalysis<CurrentSM>;

private:
  Globals& globals;
  DfResults dfResults;
  BReporter breporter;
  Transition transition;

public:
  StateMachine(Module& M_, Globals& globals_)
      : globals(globals_), breporter(globals_, dfResults),
        transition(M_, globals_, breporter) {}

  void analyze(Function* function) {
    breporter.initUnit(function);

    DfAnalysis dfAnalysis(function, dfResults, *this);

#ifdef DBGMODE
    // dfResults.print(errs());
#endif

    breporter.report();
  }

  void initLatticeValues(AbstractState& state) {
    transition.initLatticeValues(state);
  }

  bool handleInstruction(Instruction* i, AbstractState& state,
                         const Context& context) {
    return transition.handleInstruction(i, state, context);
  }

  bool isIpInstruction(Instruction* i) const {
    return globals.isIpInstruction(i);
  }

  auto& getAnalyzedFunctions() { return globals.getAnalyzedFunctions(); }
};

} // namespace llvm