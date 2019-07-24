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
    return instrType == FlushInstr || FlushInstr == FlushFenceInstr;
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

  // for conveniently propagating changes
  std::map<Variable*, Vars> objToFields;

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

  void insertPair(Variable* data, Variable* valid, Variable* obj, bool useDcl) {
    variables.insert(data);
    variables.insert(valid);

    auto [pvIt, _] = pairVariables.emplace(data, valid, useDcl);
    assert(pvIt != pairVariables.end());
    auto* pairVariable = (PairVariable*)&(*pvIt);

    varToPairs[data].insert(pairVariable);
    varToPairs[valid].insert(pairVariable);

    dataSet.insert(data);
    validSet.insert(valid);

    objToFields[obj].insert(data);
    objToFields[obj].insert(valid);
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
    assert(varToPairs.count(var));
    return varToPairs[var];
  }

  bool isData(Variable* var) const { return dataSet.count(var); }

  bool isUsedObj(Variable* var) const { return objToFields.count(var); }

  auto& getObjFields(Variable* var) {
    assert(objToFields.count(var));
    return objToFields[var];
  }

  void print(raw_ostream& O) const {
    O << "function: " << function->getName() << "\n";

    O << "variables: ";
    for (auto& variable : variables) {
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

    O << "objects: ";
    for (auto& [obj, fields] : objToFields) {
      O << obj->getName() << "-->";
      for (auto* field : fields) {
        O << field->getName() << ", ";
      }
      O << " | ";
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

  void insertPair(Variable* data, Variable* valid, Variable* obj, bool useDcl) {
    assert(activeFunction);
    activeFunction->insertPair(data, valid, obj, useDcl);
  }

  void insertInstruction(InstrType instrType, Instruction* instr,
                         Variable* var) {
    assert(activeFunction);
    activeFunction->insertInstruction(instrType, instr, var);
  }

  auto* getInstructionInfo(Instruction* i) {
    assert(activeFunction);
    return activeFunction->getInstructionInfo(i);
  }

  bool isData(Variable* var) const { return activeFunction->isData(var); }

  bool isUsedObj(Variable* var) const { return activeFunction->isUsedObj(var); }

  auto& getObjFields(Variable* var) {
    return activeFunction->getObjFields(var);
  }
};

} // namespace llvm