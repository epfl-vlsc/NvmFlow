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
    IpInstr
  };

  static constexpr const char* iStrs[] = {"write", "logging", "txbegin",
                                          "txend", "ip"};
  InstructionType iType;
  Instruction* instruction;
  Variable* variable;

  auto getName() const {
    auto name = std::string("info: ") + iStrs[(int)iType] + " ";
    if (variable) {
      name += variable->getName();
    }
    return name;
  }

  bool isIpInstr() const { return iType == IpInstr; }

  void print(raw_ostream& O) const { O << this->getName(); }

  auto* getVariable() { return variable; }

  auto* getInstruction() { return instruction; }

  static bool isLoggingBasedInstr(InstructionType instrType) {
    return instrType == LoggingInstr;
  }
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
                         Variable* variable) {
    instrToInfo[instr] = {instrType, instr, variable};
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
      O << "\t" << DbgInstr::getSourceLocation(i) << " " << ii.getName()
        << "\n";
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

  void insertInstruction(InstrType instrType, Instruction* instr,
                         Variable* var) {
    assert(activeFunction);
    activeFunction->insertInstruction(instrType, instr, var);
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