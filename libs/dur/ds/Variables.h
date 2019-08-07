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
    PfenceInstr,
    IpInstr
  };

  static constexpr const char* iStrs[] = {"write",  "flush",  "flushfence",
                                          "vfence", "pfence", "ip"};
  InstructionType iType;
  Instruction* instruction;
  SingleVariable* variable;
  SingleVariable* loadedVariable;

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

  auto* getLoadedVariable() {
    assert(loadedVariable);
    return loadedVariable;
  }

  auto* getInstruction() {
    assert(instruction);
    return instruction;
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
  std::set<SingleVariable*> annots;
  std::map<Variable*, std::set<SingleVariable*>> tracked;

  // alias info
  AliasSetTracker* ast;

  // active function
  Function* function;

  // used for lattice
  Vars variables;

  // used for instruction processing
  std::map<Instruction*, InstructionInfo> instrToInfo;

public:
  ~FunctionVariables() {
    assert(ast);
    delete ast;
  }

  void setAliasSetTracker(AliasSetTracker* ast_) { ast = ast_; }

  void setFunction(Function* function_) {
    assert(function_);
    function = function_;
  }

  bool isUsedInstruction(Instruction* instr) const {
    return instrToInfo.count(instr) > 0;
  }

  void insertVariable(Variable* var) { variables.insert(var); }

  auto* insertSingleVariable(Type* objType, StructElement* se, bool isAnnotated,
                             AliasSet* aliasSet) {
    auto varInfo = SingleVariable::getVarInfo(se, isAnnotated);
    auto [sit, _] = singleVariables.emplace(objType, se, varInfo, aliasSet);
    auto* svar = (SingleVariable*)&(*sit);
    if (isAnnotated) {
      annots.insert(svar);
    } else {
      assert(aliasSet);
      tracked[aliasSet].insert(svar);
    }

    return svar;
  }

  bool isIpInstruction(Instruction* instr) const {
    if (instrToInfo.count(instr)) {
      auto& info = instrToInfo.at(instr);
      return info.isIpInstr();
    }
    return false;
  }

  void insertInstruction(InstrType instrType, Instruction* instr,
                         SingleVariable* var, SingleVariable* loadedVar) {
    instrToInfo[instr] = {instrType, instr, var, loadedVar};
  }

  auto* getInstructionInfo(Instruction* i) {
    if (instrToInfo.count(i)) {
      return (InstructionInfo*)&instrToInfo[i];
    }
    return (InstructionInfo*)nullptr;
  }

  auto& getVariables() { return variables; }

  auto* getAliasSetTracker() { return ast; }

  bool inVars(Variable* var) const { return variables.count(var); }

  bool inAnnots(SingleVariable* var) const { return annots.count(var); }

  void print(raw_ostream& O) const {
    O << "function: " << function->getName() << "\n";

    O << "variables:";
    for (auto& variable : variables) {
      variable->print(O);
    }
    O << "\n";

    O << "inst to vars:---\n";
    for (auto& [i, ii] : instrToInfo) {
      O << "\t" << DbgInstr::getSourceLocation(i) << " " << ii.getName()
        << "\n";
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

  void setAliasSetTracker(AliasSetTracker* ast) {
    assert(activeFunction);
    return activeFunction->setAliasSetTracker(ast);
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

  auto* insertSingleVariable(Type* objType, StructElement* se, bool isAnnotated,
                             AliasSet* aliasSet) {
    assert(activeFunction);
    return activeFunction->insertSingleVariable(objType, se, isAnnotated,
                                                aliasSet);
  }

  void insertInstruction(InstrType instrType, Instruction* instr,
                         SingleVariable* var, SingleVariable* loadedVar) {
    assert(activeFunction);
    activeFunction->insertInstruction(instrType, instr, var, loadedVar);
  }

  auto* getInstructionInfo(Instruction* i) {
    assert(activeFunction);
    return activeFunction->getInstructionInfo(i);
  }

  auto* getAliasSetTracker() {
    assert(activeFunction);
    return activeFunction->getAliasSetTracker();
  }

  bool inVars(Variable* var) const { return activeFunction->inVars(var); }

  bool inAnnots(SingleVariable* var) const {
    return activeFunction->inAnnots(var);
  }
};

} // namespace llvm