#pragma once
#include "Common.h"

#include "data_util/VariableInfo.h"

namespace llvm {

class Variables {
protected:
  std::set<VariableInfo> variables;
  std::map<Value*, VariableInfo*> instructions;

public:
  void insertVariable(Instruction* i, StructType* st, unsigned idx) {
    VariableInfo vi{st, idx};
    auto [vit, _] = variables.insert(vi);
    instructions[i] = (VariableInfo*)&(*vit);
  }

  VariableInfo* getVariableFromInstr(Instruction* i) {
    if (instructions.count(i)) {
      return instructions[i];
    }

    return nullptr;
  }

  void printVariables(raw_ostream& O) const {
    O << "variables: ";
    for (auto& vi : variables) {
      O << vi.getName() << ", ";
    }
    O << "\n";

    O << "inst to vars: ";
    for (auto& [i, vi] : instructions) {
      O << *i << ":" << vi->getName() << "\n";
    }
    O << "\n";
  }

  bool isAnalyzedVariable(VariableInfo* v) { return variables.count(*v); }
};

} // namespace llvm