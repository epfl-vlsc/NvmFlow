#pragma once
#include "Common.h"
#include "Variable.h"
#include "data_util/DbgInfo.h"

namespace llvm {
/*
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

  static bool isFlushBasedInstr(InstructionType instrType) {
    return instrType == FlushInstr || instrType == FlushFenceInstr;
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

  // find variables related to field
  std::set<PairVariable> pairVariables;
  std::map<Variable*, std::set<PairVariable*>> varToPairs;

  // used elements
  std::set<Variable*> dataSet;
  std::set<Variable*> validSet;

  std::map<Variable*, Vars> writeObjMap;
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

  void insertObj(Variable* obj) {
    variables.insert(obj);

    varToPairs[obj];

    dataSet.insert(obj);
  }

  auto& getDataSet() { return dataSet; }

  auto& getValidSet() { return validSet; }

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

  void insertPair(Variable* data, Variable* valid, bool useDcl) {
    assert(activeFunction);
    activeFunction->insertPair(data, valid, useDcl);
  }

  void insertInstruction(InstrType instrType, Instruction* instr,
                         Variable* var) {
    assert(activeFunction);
    activeFunction->insertInstruction(instrType, instr, var);
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
 */
} // namespace llvm