#pragma once
#include "Common.h"
#include "Variable.h"
#include "data_util/DbgInfo.h"

namespace llvm {

struct InstructionInfo {
  enum InstructionType {
    WriteInstr,
    LoggingInstr,
    TxBegInstr,
    TxEndInstr,
    IpInstr,
    None
  };

  static constexpr const char* Strs[] = {"write", "logging", "txbegin",
                                         "txend", "ip",      "none"};
  Instruction* instruction;
  InstructionType instrType;
  Variable* variable;
  DILocalVariable* objName;

  InstructionInfo() : instrType(None) {}

  InstructionInfo(Instruction* instruction_, InstructionType instrType_,
                  Variable* variable_, DILocalVariable* objName_)
      : instruction(instruction_), instrType(instrType_), variable(variable_),
        objName(objName_) {
    assert(instruction);
    assert(instrType != None);
  }

  auto getObjName() const {
    std::string name;
    name.reserve(25);
    if (objName) {
      name += objName->getName();
    } else {
      name += "this";
    }
    return name;
  }

  auto getVariableName() const {
    std::string name;
    if (!variable)
      return name;

    name.reserve(100);
    name += getObjName();
    if (variable->isField()) {
      name += "->" + variable->getFieldName();
    }
    return name;
  }

  auto getInstructionName() const {
    assert(instruction);
    return DbgInstr::getSourceLocation(instruction);
  }

  auto getName() const {
    assert(instruction);
    std::string name;
    name.reserve(100);
    name += getInstructionName() + " ";
    name += std::string(Strs[(int)instrType]) + " ";
    name += getVariableName();
    return name;
  }

  auto getInstrType() const {
    assert(instruction);
    return instrType;
  }

  bool isIpInstr() const {
    assert(instruction);
    return instrType == IpInstr;
  }

  void print(raw_ostream& O) const { O << this->getName(); }

  auto* getVariable() {
    assert(variable);
    return variable;
  }

  auto* getInstruction() {
    assert(instruction);
    return instruction;
  }

  static bool isLoggingBasedInstr(InstructionType instrType) {
    return instrType == LoggingInstr;
  }

  static bool isUsedInstr(InstructionType it) { return it != None; }
};

class FunctionVariables {
public:
  using Vars = std::set<Variable*>;
  using InstrType = InstructionInfo::InstructionType;

private:
  // active function
  Function* function;

  // used for lattice
  Vars variables;

  // used for instruction processing
  std::map<Instruction*, InstructionInfo> instrToInfo;

  std::map<Variable*, Vars> flushFieldMap;

public:
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

  void insertVariable(Variable* data) { variables.insert(data); }

  void insertObj(Variable* obj) { variables.insert(obj); }

  void insertFlushField(Variable* obj, Variable* field) {
    assert(obj);
    auto& flushFields = flushFieldMap[obj];

    if (obj)
      flushFields.insert(field);
  }

  void insertInstruction(InstrType instrType, Instruction* instr,
                         Variable* variable, DILocalVariable* diVar) {
    instrToInfo[instr] = {instr, instrType, variable, diVar};
  }

  auto* getInstructionInfo(Instruction* i) {
    if (instrToInfo.count(i)) {
      return (InstructionInfo*)&instrToInfo[i];
    }
    return (InstructionInfo*)nullptr;
  }

  auto& getVariables() { return variables; }

  bool inVars(Variable* var) const { return variables.count(var); }

  auto& getFlushFields(Variable* obj) {
    assertInDs(flushFieldMap, obj);
    return flushFieldMap[obj];
  }

  void print(raw_ostream& O) const {
    O << "function: " << function->getName() << "\n";

    O << "variables: ";
    for (auto& variable : variables) {
      O << variable->getName() << ", ";
    }
    O << "\n";

    O << "inst to vars:---\n";
    for (auto& [i, ii] : instrToInfo) {
      O << "\t" << ii.getName() << "\n";
    }

    O << "flush fields---\n";
    for (auto& [obj, fields] : flushFieldMap) {
      O << obj->getName() << " <-> ";
      for (auto* field : fields) {
        O << field->getName() << ", ";
      }
      O << "\n";
    }
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

  bool isUsedInstruction(Instruction* i) const {
    assert(activeFunction);
    return activeFunction->isUsedInstruction(i);
  }

  void insertVariable(Variable* data) {
    assert(activeFunction);
    activeFunction->insertVariable(data);
  }

  void insertInstruction(Instruction* instr, InstrType instrType, Variable* var,
                         DILocalVariable* diVar) {
    assert(activeFunction);
    activeFunction->insertInstruction(instrType, instr, var, diVar);
  }

  void insertInstruction(Instruction* instr, InstrType instrType) {
    assert(activeFunction);
    activeFunction->insertInstruction(instrType, instr, nullptr, nullptr);
  }

  void insertFlushField(Variable* obj, Variable* field) {
    assert(activeFunction);
    activeFunction->insertFlushField(obj, field);
  }

  auto* getInstructionInfo(Instruction* i) {
    assert(activeFunction);
    return activeFunction->getInstructionInfo(i);
  }

  void insertObj(Variable* obj) {
    assert(activeFunction);
    return activeFunction->insertObj(obj);
  }

  bool inVars(Variable* var) const { return activeFunction->inVars(var); }

  auto& getFlushFields(Variable* var) {
    return activeFunction->getFlushFields(var);
  }
};

} // namespace llvm