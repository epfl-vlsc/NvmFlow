#pragma once
#include "Common.h"

//#include "Report.h"
//#include "Transfer.h"
#include "FlowTypes.h"
#include "Lattice.h"
#include "ds/Units.h"

#include "analysis_util/DataflowAnalysis.h"

namespace llvm {

class StateMachine {
private:
  Units& units;

public:
  using AbstractState = AbstractState;

  StateMachine(Units& units_) : units(units_) {}

  void setUnit(Function* function) {}

  void analyze(Function* function) {
    units.setActiveFunction(function);
    units.printActiveFunction(llvm::errs());

    // DataflowAnalysis dataflow(function, *this);
  }

  void initLatticeValues(AbstractState& state) {
    // transfer.initLatticeValues(state);
  }

  bool handleInstruction(Instruction* i, AbstractState& state) {
    // return transfer.handleStmt(i, state);
    return false;
  }

  bool isIpaInstruction(Instruction* i) const {
    return units.activeFunction->isCallInstruction(i);
  }

  auto& getAnalyzedFunctions() { return units.getAnalyzedFunctions(); }
};

} // namespace llvm