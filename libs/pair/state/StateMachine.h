#pragma once
#include "Common.h"

#include "BugReporter.h"
#include "FlowTypes.h"
#include "Lattice.h"
#include "Transfer.h"
#include "analysis_util/DataflowAnalysis.h"
#include "ds/Units.h"

namespace llvm {

class StateMachine {
public:
  using AbstractState = AbstractState;

private:
  Units& units;
  BugReporter breporter;
  Transfer transfer;

public:
  StateMachine(Module& M_, Units& units_)
      : units(units_), breporter(units_), transfer(M_, units_, breporter) {}

  void analyze(Function* function) {
    units.setActiveFunction(function);

    errs() << "\n\n";

#ifdef DBGMODE
    units.printVariables(errs());
#endif

    breporter.initUnit(function);

    DataflowAnalysis dataflow(function, *this);

#ifdef DBGMODE
    dataflow.print(errs());
#endif

    breporter.print(errs());
  }

  void initLatticeValues(AbstractState& state) {
    transfer.initLatticeValues(state);
  }

  bool handleInstruction(Instruction* i, AbstractState& state) {
    return transfer.handleInstruction(i, state);
  }

  bool isIpInstruction(Instruction* i) const {
    return units.isIpInstruction(i);
  }

  auto& getAnalyzedFunctions() { return units.getAnalyzedFunctions(); }
};

} // namespace llvm