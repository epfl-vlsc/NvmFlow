#pragma once
#include "Common.h"
#include "Variable.h"
#include "ds/InstrInfo.h"

namespace llvm {

struct UnitInfo {
  using VarSet = std::set<Variable*>;

  // active function
  Function* func;

  // lattice variables
  std::set<Variable> vars;

  // info gathered for instruction
  std::map<Instruction*, InstrInfo> iiMap;

  void print(raw_ostream& O) const;
};

class Locals {
  using InstrType = typename InstrInfo::InstrType;

  std::map<Function*, UnitInfo> unitMap;
  UnitInfo* activeUnit;

public:
  Locals() : activeUnit(nullptr) {}

  void print(raw_ostream& O) const {
    assert(activeUnit);
    activeUnit->print(O);
  }

  Variable* addVariable(int i) {
    assert(activeUnit);
    auto var = Variable(i);
    auto& vars = activeUnit->vars;
    auto [varsIt, _] = vars.insert(var);
    assert(varsIt != vars.end());
    auto* varPtr = (Variable*)&(*varsIt);
    return varPtr;
  }

  Variable* getVariable(int i) {
    assert(activeUnit);
    auto var = Variable(i);
    auto& vars = activeUnit->vars;
    assert(vars.count(var));
    auto varIt = vars.find(var);
    assert(varIt != vars.end());
    auto varPtr = (Variable*)&(*varIt);
    return varPtr;
  }

  void addInstrInfo(Instruction* i, InstrType instrType, Variable* lhs,
                    Variable* rhs, ParsedVariable& pvLhs,
                    ParsedVariable& pvRhs) {
    assert(activeUnit);
    auto& iiMap = activeUnit->iiMap;
    auto ii = InstrInfo(i, instrType, lhs, rhs, pvLhs, pvRhs);

    iiMap[i] = ii;
  }

  auto& getVariables() {
    assert(activeUnit);
    return activeUnit->vars;
  }

  void setFunction(Function* function) {
    assert(function);
    activeUnit = &unitMap[function];
    activeUnit->func = function;
  }

  bool isIpInstruction(Instruction* i) const {
    assert(activeUnit);
    auto& iiMap = activeUnit->iiMap;

    if (iiMap.count(i)) {
      auto& ii = iiMap[i];
      return ii.isIpInstr();
    }
    return false;
  }

  bool isUsedInstruction(Instruction* i) const {
    assert(activeUnit);
    auto& iiMap = activeUnit->iiMap;

    if (iiMap.count(i)) {
      auto& ii = iiMap[i];
      return ii.isUsedInstr();
    }
    return false;
  }

  auto* getInstrInfo(Instruction* i) {
    assert(activeUnit);
    auto& iiMap = activeUnit->iiMap;

    if (iiMap.count(i)) {
      auto& ii = iiMap[i];
      return &ii;
    }

    return (InstrInfo*)nullptr;
  }
};

void UnitInfo::print(raw_ostream& O) const {
  O << "Unit Info\n";
  O << "---------\n";

  O << "function: " << func->getName() << "\n";

  O << "variables:---\n";
  for (auto& var : vars) {
    O << var.getName() << "\n";
  }

  O << "inst to vars sample:---\n";
  for (auto& [i, ii] : iiMap) {
    O << "\t";
    ii.print(O);
  }
}

} // namespace llvm