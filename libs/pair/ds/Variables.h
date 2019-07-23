#pragma once
#include "Common.h"
#include "Variable.h"
#include "data_util/DbgInfo.h"

namespace llvm {

struct InstructionInfo {
  enum InstructionType {
    WriteInstr,
    FlushInstr,
    FlushFenceInstr,
    VfenceInstr,
    PfenceInstr,
    IpInstr
  };

  static constexpr const char* iStrs[] = {"write",  "flush",  "flushfence",
                                          "vfence", "pfence", "ip"};
  InstructionType iType;
  Instruction* instruction;
  StructElement* se;

  auto getName() const {
    auto name = std::string("info: ") + iStrs[(int)iType] + " ";
    if (se) {
      name += se->getName();
    }
    return name;
  }

  bool isIpInstr() const { return iType == IpInstr; }

  void print(raw_ostream& O) const { O << this->getName(); }

  auto* getVariable() { return se; }

  auto* getInstruction() { return instruction; }
};

class FunctionVariables {
public:
  using LatticeVariables = std::set<Variable*>;
  using InstrType = InstructionInfo::InstructionType;

private:
  // active function
  Function* function;

  // used for lattice
  LatticeVariables variables;

  // used for instruction processing
  std::map<Instruction*, InstructionInfo> instrToInfo;

  // find variables related to field
  std::map<StructElement*, LatticeVariables> seToVariables;

  // used elements
  std::set<StructElement*> dataSet;
  std::set<StructElement*> validSet;

public:
  ~FunctionVariables() {
    for (auto* variable : variables) {
      delete variable;
    }
  }

  void setFunction(Function* function_) {
    assert(function_);
    function = function_;
  }

  bool isUsedInstruction(Instruction* instr) const {
    return instrToInfo.count(instr) > 0;
  }

  bool isIpInstruction(Instruction* instr) const {
    if (instrToInfo.count(instr)) {
      auto& info = instrToInfo.at(instr);
      return info.isIpInstr();
    }
    return false;
  }

  auto* insertVariable(StructElement* data, StructElement* valid, bool useDcl) {
    auto* variable = new Variable(data, valid, useDcl);
    variables.insert(variable);

    seToVariables[data].insert(variable);
    seToVariables[valid].insert(variable);

    dataSet.insert(data);
    validSet.insert(valid);
    return variable;
  }

  void insertAffectedField() {
    // todo
  }

  void insertAffectedObj() {
    // todo
  }

  void insertInstruction(InstrType instrType, Instruction* instr,
                         StructElement* se) {
    instrToInfo[instr] = {instrType, instr, se};
  }

  auto* getInstructionInfo(Instruction* i) {
    if (instrToInfo.count(i)) {
      return (InstructionInfo*)&instrToInfo[i];
    }
    return (InstructionInfo*)nullptr;
  }

  auto& getVariables() { return variables; }

  bool isData(StructElement* se) const{
    return dataSet.count(se);
  }

  void print(raw_ostream& O) const {
    O << "function: " << function->getName() << "\n";

    O << "variables: ";
    for (auto* variable : variables) {
      O << variable->getName() << ", ";
    }
    O << "\n";

    O << "inst to vars:\n";
    for (auto& [i, ii] : instrToInfo) {
      O << "\t" << DbgInstr::getSourceLocation(i) << " " << ii.getName()
        << "\n";
    }

    O << "data: ";
    for (auto* se : dataSet) {
      O << se->getName() << ", ";
    }
    O << "\n";

    O << "valid: ";
    for (auto* se : validSet) {
      O << se->getName() << ", ";
    }
    O << "\n";
  }
};

class Variables {
  using InstrType = typename FunctionVariables::InstrType;
  std::map<Function*, FunctionVariables> funcVars;
  FunctionVariables* activeFunction;

public:
  Variables() : activeFunction(nullptr) {}

  void print(raw_ostream& O) const {
    assert(activeFunction);
    activeFunction->print(O);
  }

  auto& getVariables() {
    assert(activeFunction);
    return activeFunction->getVariables();
  }

  void setFunction(Function* function) {
    assert(function);
    activeFunction = &funcVars[function];
    activeFunction->setFunction(function);
  }

  bool isIpInstruction(Instruction* i) const {
    assert(activeFunction);
    return activeFunction->isIpInstruction(i);
  }

  auto* insertVariable(StructElement* data, StructElement* valid, bool useDcl) {
    return activeFunction->insertVariable(data, valid, useDcl);
  }

  void insertInstruction(InstrType instrType, Instruction* instr,
                         StructElement* se) {
    activeFunction->insertInstruction(instrType, instr, se);
  }

  bool isData(StructElement* se) const{
    return activeFunction->isData(se);
  }
};

} // namespace llvm