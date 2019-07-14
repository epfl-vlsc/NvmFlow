#pragma once
#include "Common.h"

#include "data_util/Variable.h"

namespace llvm {

struct InstructionInfo {
  enum InstructionType {
    LoggingInstr,
    WriteInstr,
    CallInstr,
    TxBegInstr,
    TxEndInstr
  };

  static constexpr const char* iStrs[] = {"logging", "write", "call", "txbegin",
                                          "txend"};
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

  void print(raw_ostream& O) const { O << this->getName(); }
};

class FunctionVariables {
  using AllVariables = std::set<Variable>;
  using Variables = std::set<Variable*>;
  using InstrType = InstructionInfo::InstructionType;

  // active function
  Function* function;

  // stores objects as well
  AllVariables allVariables;

  // used for lattice
  Variables variables;

  // used for instruction processing
  std::map<Instruction*, InstructionInfo> instrToInfo;
  std::map<StructType*, Variables> affectedVariables;

public:
  void setFunction(Function* function_) { function = function_; }

  bool isUsedInstruction(Instruction* instr) const {
    return instrToInfo.count(instr) > 0;
  }

  auto insertVariable(StructType* st, int idx) {
    auto [varPtr, _] = allVariables.emplace(st, idx);
    auto* variable = (Variable*)&(*varPtr);
    return variable;
  }

  void insertInstruction(InstrType instrType, Instruction* instr,
                         Variable* variable) {
    if (variable && !variable->useObj()) {
      variables.insert(variable);
      auto* st = variable->getType();
      affectedVariables[st].insert(variable);
    }
    instrToInfo[instr] = {instrType, instr, variable};
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

    O << "affected variables:\n";
    for (auto& [st, variables] : affectedVariables) {
      O << st->getName() << ": ";
      for (auto variable : variables) {
        O << variable->getName() << ",";
      }
      O << "\n";
    }
  }
};

} // namespace llvm