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
    IpInstr,
    None
  };

  static constexpr const char* Strs[] = {
      "write", "flush", "flushfence", "vfence", "pfence", "ip", "none"};
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

  static bool isFlushBasedInstr(InstructionType instrType) {
    return instrType == FlushInstr || instrType == FlushFenceInstr;
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

  // find variables related to field
  std::set<PairVariable> pairVariables;
  std::map<Variable*, std::set<PairVariable*>> varToPairs;

  // used elements
  std::set<Variable*> dataSet;
  std::set<Variable*> validSet;

  std::map<Variable*, Vars> writeObjMap;
  std::map<Variable*, Vars> flushFieldMap;

  std::map<Instruction*, DILocalVariable*> localVars;

public:
  void setFunction(Function* function_) {
    assert(function_);
    function = function_;
  }

  bool isUsedInstruction(Instruction* instr) const {
    return instrToInfo.count(instr) > 0;
  }

  bool instrHasLocalVar(Instruction* instr) const {
    return localVars.count(instr) > 0;
  }

  bool isIpInstruction(Instruction* instr) const {
    if (instrToInfo.count(instr)) {
      auto& info = instrToInfo.at(instr);
      return info.isIpInstr();
    }
    return false;
  }

  void insertPair(Variable* data, Variable* valid, bool useDcl) {
    variables.insert(data);
    variables.insert(valid);

    auto [pvIt, _] = pairVariables.emplace(data, valid, useDcl);
    assert(pvIt != pairVariables.end());
    auto* pairVariable = (PairVariable*)&(*pvIt);

    varToPairs[data].insert(pairVariable);
    varToPairs[valid].insert(pairVariable);

    dataSet.insert(data);
    validSet.insert(valid);
  }

  void insertLocalVariable(Instruction* i, DILocalVariable* diVar) {
    localVars[i] = diVar;
  }

  void insertObj(Variable* obj) {
    variables.insert(obj);

    varToPairs[obj];

    dataSet.insert(obj);
  }

  auto& getDataSet() { return dataSet; }

  auto& getValidSet() { return validSet; }

  auto* getLocalVar(Instruction* i) { return localVars[i]; }

  void insertWriteObj(Variable* field, Variable* obj) {
    assert(field);
    auto& writeObjs = writeObjMap[field];

    if (obj)
      writeObjs.insert(obj);
  }

  void insertFlushField(Variable* obj, Variable* field) {
    assert(obj);
    auto& flushFields = flushFieldMap[obj];

    if (obj)
      flushFields.insert(field);
  }

  void insertInstruction(Instruction* instr, InstrType instrType, Variable* var,
                         DILocalVariable* localVar) {
    instrToInfo[instr] = {instr, instrType, var, localVar};
  }

  auto* getInstructionInfo(Instruction* i) {
    if (instrToInfo.count(i)) {
      return (InstructionInfo*)&instrToInfo[i];
    }
    return (InstructionInfo*)nullptr;
  }

  auto& getVariables() { return variables; }

  auto& getPairs(Variable* var) {
    assertInDs(varToPairs, var);
    return varToPairs[var];
  }

  bool inDataSet(Variable* var) const { return dataSet.count(var); }

  bool inValidSet(Variable* var) const { return validSet.count(var); }

  bool inVars(Variable* var) const { return variables.count(var); }

  auto& getWriteObjs(Variable* field) {
    assertInDs(writeObjMap, field);
    return writeObjMap[field];
  }

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

    O << "pair variables: ";
    for (auto& pair : pairVariables) {
      O << pair.getName() << ", ";
    }
    O << "\n";

    O << "inst to vars:---\n";
    for (auto& [i, ii] : instrToInfo) {
      O << "\t" << ii.getName() << "\n";
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

    O << "flush fields---\n";
    for (auto& [obj, fields] : flushFieldMap) {
      O << obj->getName() << " <-> ";
      for (auto* field : fields) {
        O << field->getName() << ", ";
      }
      O << "\n";
    }

    O << "write objs---\n";
    for (auto& [field, objs] : writeObjMap) {
      O << field->getName() << " <-> ";
      for (auto* obj : objs) {
        O << obj->getName() << ", ";
      }
      O << "\n";
    }

    O << "local vars---\n";
    for (auto& [i, var] : localVars) {
      O << *i << " <=> " << var->getName() << "\n";
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

  auto& getPairs(Variable* var) {
    assert(activeFunction);
    return activeFunction->getPairs(var);
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

  bool instrHasLocalVar(Instruction* i) const {
    assert(activeFunction);
    return activeFunction->instrHasLocalVar(i);
  }

  auto* getLocalVar(Instruction* i) {
    assert(activeFunction);
    return activeFunction->getLocalVar(i);
  }

  void insertPair(Variable* data, Variable* valid, bool useDcl) {
    assert(activeFunction);
    activeFunction->insertPair(data, valid, useDcl);
  }

  void insertInstruction(Instruction* instr, InstrType instrType, Variable* var,
                         DILocalVariable* diVar) {
    assert(activeFunction);
    activeFunction->insertInstruction(instr, instrType, var, diVar);
  }

  void insertLocalVariable(Instruction* instr, DILocalVariable* diVar) {
    assert(activeFunction);
    activeFunction->insertLocalVariable(instr, diVar);
  }

  void insertInstruction(Instruction* instr, InstrType instrType) {
    assert(activeFunction);
    activeFunction->insertInstruction(instr, instrType, nullptr, nullptr);
  }

  void insertWriteObj(Variable* field, Variable* obj) {
    assert(activeFunction);
    activeFunction->insertWriteObj(field, obj);
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

  bool inDataSet(Variable* var) const { return activeFunction->inDataSet(var); }

  bool inValidSet(Variable* var) const {
    return activeFunction->inValidSet(var);
  }

  bool inVars(Variable* var) const { return activeFunction->inVars(var); }

  auto& getDataSet() { return activeFunction->getDataSet(); }

  auto& getValidSet() { return activeFunction->getValidSet(); }

  auto& getWriteObjs(Variable* var) {
    return activeFunction->getWriteObjs(var);
  }

  auto& getFlushFields(Variable* var) {
    return activeFunction->getFlushFields(var);
  }
};

} // namespace llvm