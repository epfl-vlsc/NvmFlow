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
  bool isData;

  auto getName() const {
    auto name = std::string("info: ") + iStrs[(int)iType] + " ";
    if (variable) {
      auto* usedVar = variable->getUsed(isData);
      name += usedVar->getName() + " " + variable->getName();
    }
    return name;
  }

  bool isIpInstr() const { return iType == IpInstr; }

  void print(raw_ostream& O) const { O << this->getName(); }

  auto* getVariable() { return variable; }

  auto* getInstruction() { return instruction; }

  bool isDataVar() const { return isData; }
};

class FunctionVariables {
  using Item = FullStructElement;
  using LatticeVariables = std::set<Variable*>;
  using InstrType = InstructionInfo::InstructionType;

  // active function
  Function* function;

  // used for lattice
  LatticeVariables variables;

  // used for instruction processing
  std::map<Instruction*, InstructionInfo> instrToInfo;

public:
  void setFunction(Function* function_) { function = function_; }

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

  auto* insertVariable(Item* data, Item* valid, bool useDcl) {
    auto* variable = new Variable(data, valid, useDcl);
    variables.insert(variable);
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
    for (auto* variable : variables) {
      O << variable->getName() << ", ";
    }
    O << "\n";

    O << "inst to vars:\n";
    for (auto& [i, ii] : instrToInfo) {
      O << "\t" << ii.getName() << "\n";
    }
    /*
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
      */
  }
};

class Variables {
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
    assert(activeFunction);
    activeFunction = &funcVars[function];
    activeFunction->setFunction(function);
  }

  bool isIpInstruction(Instruction* i) const {
    assert(activeFunction);
    activeFunction->isIpInstruction(i);
  }
};

} // namespace llvm