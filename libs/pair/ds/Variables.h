#pragma once
#include "Common.h"

#include "Variable.h"

namespace llvm {

struct InstructionInfo {
  enum InstructionType {
    WriteInstr,
    FlushInstr,
    FlushFenceInstr,
    VfenceInstr,
    PfenceInstr,
    CallInstr
  };

  static constexpr const char* iStrs[] = {"write",  "flush",  "flushfence",
                                          "vfence", "pfence", "call"};
  InstructionType iType;
  Instruction* instruction;
  Variable* variable;
  bool isData;

  auto getName() const {
    auto name = std::string("info: ") + iStrs[(int)iType] + " ";
    if (variable) {
      auto* usedVar = variable->getUsed(isData);
      name += usedVar->getName() + " " + variable->getName();
    }
    return name;
  }

  bool isCallInstr() const { return iType == CallInstr; }

  void print(raw_ostream& O) const { O << this->getName(); }

  auto* getVariable() { return variable; }

  auto* getInstruction() { return instruction; }

  bool isDataVar() const { return isData; }
};

class FunctionVariables {
  using AllSingles = std::set<SingleVariable>;
  using AllVariables = std::set<Variable>;
  using Variables = std::set<Variable*>;
  using InstrType = InstructionInfo::InstructionType;

  // active function
  Function* function;

  // stores objects as well
  AllSingles allSingles;
  AllVariables allVariables;

  // used for lattice
  Variables variables;

  // used for instruction processing
  using Structs = std::set<StructType*>;
  std::map<Instruction*, InstructionInfo> instrToInfo;
  std::map<StructType*, Variables> affectedFields;
  std::map<Variable*, Variables> affectedObjs;

  auto* insertSingleVariable(StructType* st, int idx) {
    auto [singlePtr, _] = allSingles.emplace(st, idx);
    auto* single = (SingleVariable*)&(*singlePtr);
    return single;
  }

public:
  void setFunction(Function* function_) { function = function_; }

  auto& getAffectedFields(StructType* st) { return affectedFields[st]; }

  auto& getAffectedObjs(Variable* st) { return affectedObjs[st]; }

  bool isUsedInstruction(Instruction* instr) const {
    return instrToInfo.count(instr) > 0;
  }

  bool isCallInstruction(Instruction* instr) const {
    if (instrToInfo.count(instr)) {
      auto& info = instrToInfo.at(instr);
      return info.isCallInstr();
    }
    return false;
  }

  auto insertVariable(StructType* st1, int idx1, StructType* st2, int idx2,
                      bool useDcl) {
    auto* single1 = insertSingleVariable(st1, idx1);
    auto* single2 = insertSingleVariable(st2, idx2);

    auto [varPtr, _] = allVariables.emplace(single1, single2, useDcl);
    auto* variable = (Variable*)&(*varPtr);
    return variable;
  }

  void insertAffectedField() {
    // todo
  }

  void insertAffectedObj() {
    // todo
  }

  void insertInstruction(InstrType instrType, Instruction* instr,
                         Variable* variable) {
    if (variable) {
      variables.insert(variable);
    }
    instrToInfo[instr] = {instrType, instr, variable};
  }

  auto* getInstructionInfo(Instruction* i) {
    if (instrToInfo.count(i)) {
      return (InstructionInfo*)&instrToInfo[i];
    }
    return (InstructionInfo*)nullptr;
  }

  auto& getVariables() { return variables; }

  void print(raw_ostream& O) const {
    O << "function: " << function->getName() << "\n";

    O << "variables: ";
    for (auto& var : allVariables) {
      Variable* variable = (Variable*)&var;
      bool isLatticeVariable = variables.count(variable);
      if (isLatticeVariable) {
        O << "lattice:";
      }
      O << var.getName() << ", ";
    }
    O << "\n";

    O << "inst to vars:\n";
    for (auto& [i, ii] : instrToInfo) {
      O << "\t" << ii.getName() << "\n";
    }

    O << "affected fields:\n";
    for (auto& [st, variables] : affectedFields) {
      O << st->getName() << ": ";
      for (auto variable : variables) {
        O << variable->getName() << ",";
      }
      O << "\n";
    }

    O << "affected objs:\n";
    for (auto& [variable, structs] : affectedObjs) {
      O << variable->getName() << ": ";
      for (auto st : structs) {
        O << st->getName() << ",";
      }
      O << "\n";
    }
  }
};

} // namespace llvm