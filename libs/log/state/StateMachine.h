#pragma once
#include "Common.h"

//#include "Report.h"
//
#include "BugReporter.h"
#include "FlowTypes.h"
#include "Lattice.h"
#include "Transfer.h"
#include "analysis_util/DataflowAnalysis.h"
#include "ds/Units.h"

namespace llvm {

class StateMachine {
private:
  Units& units;
  BugReporter breporter;
  Transfer transfer;

public:
  using AbstractState = AbstractState;

  StateMachine(Module& M, Units& units_)
      : units(units_), transfer(M, units_, breporter) {}

  void setUnit(Function* function) {}

  void analyze(Function* function) {
    units.setActiveFunction(function);
    units.printActiveFunction(errs());
    breporter.initUnit();

    DataflowAnalysis dataflow(function, *this);
    dataflow.print(errs());
    breporter.print(errs());
  }

  void initLatticeValues(AbstractState& state) {
    transfer.initLatticeValues(state);
  }

  bool handleInstruction(Instruction* i, AbstractState& state) {
    return transfer.handleInstruction(i, state);
  }

  bool isIpaInstruction(Instruction* i) const {
    return units.activeFunction->isCallInstruction(i);
  }

  auto& getAnalyzedFunctions() { return units.getAnalyzedFunctions(); }
};

} // namespace llvm