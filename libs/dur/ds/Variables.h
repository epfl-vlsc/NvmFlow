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

  SingleVariable* var;
  SingleVariable* loadVar;

  auto getVariableName(bool loaded = false) const {
    if (!loaded && var) {
      return var->getName();
    } else if (loaded && loadVar) {
      return loadVar->getName();
    } else {
      return std::string("");
    }
  }

  auto getInstructionName() const {
    assert(instruction);
    return DbgInstr::getSourceLocation(instruction);
  }

  auto getName() const {
    assert(instruction);
    std::string name;
    name.reserve(200);
    name += getInstructionName() + " ";
    name += std::string(Strs[(int)instrType]) + " ";
    name += getVariableName() + " ";
    name += getVariableName(true);
    return name;
  }

  bool isIpInstr() const { return instrType == IpInstr; }

  void print(raw_ostream& O) const { O << this->getName(); }

  auto* getVariable() {
    assert(var);
    return var;
  }

  auto* getLoadVariable() {
    assert(loadVar);
    return loadVar;
  }

  auto* getInstruction() {
    assert(instruction);
    return instruction;
  }

  auto getInstrType() const {
    assert(instruction);
    return instrType;
  }

  static bool isFlushBasedInstr(InstructionType instrType) {
    return instrType == FlushInstr || instrType == FlushFenceInstr;
  }
};

class FunctionVariables {
public:
  using Vars = std::set<Variable*>;
  using InstrType = InstructionInfo::InstructionType;

private:
  // store location of variables
  std::set<SingleVariable> singleVariables;

  // alias info
  AliasGroups ags;

  // active function
  Function* function;

  // used for lattice
  Vars variables;

  // used for instruction processing
  std::map<Instruction*, InstructionInfo> instrToInfo;

  // local var names
  std::map<Value*, DILocalVariable*> localVars;

public:
  void setFunction(Function* function_) {
    assert(function_);
    function = function_;
  }

  bool isUsedInstruction(Instruction* instr) const {
    return instrToInfo.count(instr) > 0;
  }

  auto* getAliasGroup(Instruction* i, bool loaded) {
    return ags.getAliasGroup(i, loaded);
  }

  void insertVariable(Variable* var) { variables.insert(var); }

  auto* insertSingleVariable(Type* t, StructElement* se, bool ann,
                             AliasGroup* ag, DILocalVariable* diVar) {
    auto vi = VarElement::getVarInfo(se, ann);
    auto [it, _] = singleVariables.emplace(t, se, vi, ag, diVar);
    auto* sv = (SingleVariable*)(&*it);
    return sv;
  }

  void insertAliasInstr(Instruction* i) { ags.add(i); }

  void insertInstruction(Instruction* instr, InstrType instrType,
                         SingleVariable* var, SingleVariable* loadVar) {
    instrToInfo[instr] = {instr, instrType, var, loadVar};
  }

  void createAliasGroups(AAResults* AAR) {
    ags.createGroups(AAR);
    for (auto& ag : ags) {
      variables.insert((AliasGroup*)&ag);
    }
  }

  bool isIpInstruction(Instruction* instr) const {
    if (instrToInfo.count(instr)) {
      auto& info = instrToInfo.at(instr);
      return info.isIpInstr();
    }
    return false;
  }

  auto* getInstructionInfo(Instruction* i) {
    if (instrToInfo.count(i)) {
      return (InstructionInfo*)&instrToInfo[i];
    }
    return (InstructionInfo*)nullptr;
  }

  auto* getLocalVar(Value* v) {
    if (localVars.count(v))
      return localVars[v];

    return (DILocalVariable*)nullptr;
  }

  void insertLocalVariable(Value* v, DILocalVariable* diVar) {
    localVars[v] = diVar;
  }

  auto& getVariables() { return variables; }

  bool inVars(Variable* var) const { return variables.count(var); }

  void print(raw_ostream& O) const {
    O << "function: " << function->getName() << "\n";

    O << "alias groups:---\n";
    ags.print(O);
    O << "\n";

    O << "variables: ";
    for (auto& variable : variables) {
      O << variable->getName() << ", ";
    }
    O << "\n";

    O << "inst to vars:---\n";
    for (auto& [i, ii] : instrToInfo) {
      O << "\t" << ii.getName() << "\n";
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

  void insertVariable(Variable* var) {
    assert(activeFunction);
    return activeFunction->insertVariable(var);
  }

  auto* getInstructionInfo(Instruction* i) {
    assert(activeFunction);
    return activeFunction->getInstructionInfo(i);
  }

  bool inVars(Variable* var) const { return activeFunction->inVars(var); }

  void insertAliasInstr(Instruction* i) {
    assert(activeFunction);
    activeFunction->insertAliasInstr(i);
  }

  auto* insertSingleVariable(Type* t, StructElement* se, bool ann,
                             AliasGroup* ag, DILocalVariable* diVar) {
    assert(activeFunction);
    return activeFunction->insertSingleVariable(t, se, ann, ag, diVar);
  }

  void createAliasGroups(AAResults* AAR) {
    assert(activeFunction);
    activeFunction->createAliasGroups(AAR);
  }

  auto* getAliasGroup(Instruction* i, bool loaded = false) {
    assert(activeFunction);
    return activeFunction->getAliasGroup(i, loaded);
  }

  void insertInstruction(Instruction* instr, InstrType instrType) {
    assert(activeFunction);
    activeFunction->insertInstruction(instr, instrType, nullptr, nullptr);
  }

  void insertInstruction(Instruction* instr, InstrType instrType,
                         SingleVariable* var) {
    assert(activeFunction);
    activeFunction->insertInstruction(instr, instrType, var, nullptr);
  }

  void insertInstruction(Instruction* instr, InstrType instrType,
                         SingleVariable* var, SingleVariable* loadVar) {
    assert(activeFunction);
    activeFunction->insertInstruction(instr, instrType, var, loadVar);
  }

  auto* getLocalVar(Value* v) {
    assert(activeFunction);
    return activeFunction->getLocalVar(v);
  }

  void insertLocalVariable(Value* v, DILocalVariable* diVar) {
    assert(activeFunction);
    activeFunction->insertLocalVariable(v, diVar);
  }
};

} // namespace llvm