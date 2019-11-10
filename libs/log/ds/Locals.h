#pragma once
#include "Common.h"
#include "Variable.h"
#include "ds/InstrInfo.h"

namespace llvm {

struct UnitInfo {
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

  Variable* addVariable(Variable& var) {
    assert(activeUnit);
    auto& vars = activeUnit->vars;
    auto [varsIt, _] = vars.insert(var);
    assert(varsIt != vars.end());
    auto* varPtr = (Variable*)&(*varsIt);
    return varPtr;
  }

  auto* addVariable(StructField* sf, int alias) {
    assert(activeUnit);
    auto var = Variable(sf, alias);
    auto* varPtr = addVariable(var);
    return varPtr;
  }

  auto* addVariable(StructType* st, int alias) {
    assert(activeUnit);
    auto var = Variable(st, alias);
    auto* varPtr = addVariable(var);
    return varPtr;
  }

  void addInstrInfo(Instruction* i, InstrType instrType, Variable* var,
                    ParsedVariable& pv) {
    assert(activeUnit);
    auto& iiMap = activeUnit->iiMap;
    auto ii = InstrInfo(i, instrType, var, pv);

    iiMap[i] = ii;
  }

  auto& getVariables() {
    assert(activeUnit);
    return activeUnit->vars;
  }

  bool inVariables(StructField* sf, int setNo) const {
    assert(activeUnit);
    auto var = Variable(sf, setNo);
    auto& vars = activeUnit->vars;
    return vars.count(var);
  }

  Variable* getVariable(Variable& var) {
    assert(activeUnit);
    auto& vars = activeUnit->vars;
    assert(vars.count(var));
    auto varIt = vars.find(var);
    assert(varIt != vars.end());
    auto varPtr = (Variable*)&(*varIt);
    return varPtr;
  }

  auto* getVariable(StructField* sf, int setNo) {
    assert(activeUnit);
    auto var = Variable(sf, setNo);
    return getVariable(var);
  }

  auto* getVariable(StructType* st, int setNo) {
    assert(activeUnit);
    auto var = Variable(st, setNo);
    return getVariable(var);
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
    var.print(O);
    O << "\n";
  }

  O << "inst to vars sample:---\n";
  for (auto& [i, ii] : iiMap) {
      ii.print(O);
  }
}

} // namespace llvm