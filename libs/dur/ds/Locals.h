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

  // aliases
  std::map<Value*, VarSet> aliases;

  // info gathered for instruction
  std::map<Instruction*, InstrInfo> iiMap;

  void print(raw_ostream& O) const;
};

class Locals {
  using InstrType = typename InstrInfo::InstrType;

  std::map<Function*, UnitInfo> unitMap;
  UnitInfo* activeUnit;
  AAResults& AAR;

public:
  Locals(AAResults& AAR_) : activeUnit(nullptr), AAR(AAR_) {}

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

  void addInstrInfo(Instruction* i, InstrType instrType, Variable* var,
                    Value* rhs) {
    assert(activeUnit);
    auto& iiMap = activeUnit->iiMap;
    auto ii = InstrInfo(i, instrType, var, rhs);

    iiMap[i] = ii;
  }

  auto& getVariables() {
    assert(activeUnit);
    return activeUnit->vars;
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

  auto* getVariable(Value* localVar, Type* type, StructField* sf, bool locRef) {
    assert(activeUnit);
    auto var = Variable::getSearchVariable(localVar, type, sf, locRef);
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
    if (ii.isFlushBasedInstr())
      O << "\t" << ii.getName() << "\n";
  }

  O << "alias samples:---\n";
  for (auto& [val, aliasSet] : aliases) {
    for (auto& var : aliasSet) {
      O << "\t" << var->getName() << "\n";
    }
  }
}

} // namespace llvm