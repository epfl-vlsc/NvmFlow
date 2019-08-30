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
  std::set<Variable*> sentinels;

  // pairs
  std::set<PairVariable> pairs;

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

  void addPair(Variable* data, Variable* valid, bool useDcl) {
    assert(activeUnit);
    auto pair = PairVariable(data, valid, useDcl);
    auto& pairs = activeUnit->pairs;
    pairs.insert(pair);
  }

  Variable* addVariable(Variable& var) {
    assert(activeUnit);
    auto& vars = activeUnit->vars;
    auto [varsIt, _] = vars.insert(var);
    assert(varsIt != vars.end());
    auto* varPtr = (Variable*)&(*varsIt);
    return varPtr;
  }

  void addSentinel(Variable* var) {
    assert(activeUnit);
    auto& sentinels = activeUnit->sentinels;
    sentinels.insert(var);
  }

  auto* addVariable(StructField* sf) {
    assert(activeUnit);
    auto var = Variable(sf);
    auto* varPtr = addVariable(var);
    return varPtr;
  }

  auto* addVariable(StructType* st) {
    assert(activeUnit);
    auto var = Variable(st);
    auto* varPtr = addVariable(var);
    return varPtr;
  }

  void addInstrInfo(Instruction* i, InstrType instrType, Variable* var,
                    ParsedVariable pv) {
    assert(activeUnit);
    auto& iiMap = activeUnit->iiMap;
    auto ii = InstrInfo(i, instrType, var, pv);

    iiMap[i] = ii;
  }

  auto& getPairs() {
    assert(activeUnit);
    return activeUnit->pairs;
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

  auto* getVariable(StructField* sf) {
    assert(activeUnit);
    auto var = Variable(sf);
    return getVariable(var);
  }

  auto* getVariable(StructType* st) {
    assert(activeUnit);
    auto var = Variable(st);
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

  bool inSentinels(Variable* var) const {
    assert(activeUnit);
    auto& sentinels = activeUnit->sentinels;
    return sentinels.count(var);
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

  O << "sentinels:";
  for (auto& sentinel : sentinels) {
    O << sentinel->getName() << ", ";
  }
  O << "\n";

  O << "pairs: ";
  for (auto& pair : pairs) {
    O << pair.getName() << ", ";
  }
  O << "\n";

  O << "inst to vars:---\n";
  for (auto& [i, ii] : iiMap) {
    O << "\t" << ii.getName() << "\n";
  }
}

} // namespace llvm