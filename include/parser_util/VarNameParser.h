#pragma once
#include "Common.h"

namespace llvm {

template <typename Units> class VarNameParser {

  void insertVarName(DbgValueInst* dvi) {
    auto* val = dvi->getValue();
    auto* var = dvi->getVariable();

    assert(var && val);
    auto* instr = dyn_cast<Instruction>(val);

    units.variables.insertLocalVariable(instr, var);
  }

  void insertVarNames(Function* function) {
    for (auto* f : units.functions.getUnitFunctions(function)) {
      for (auto& I : instructions(*f)) {
        if (auto* dvi = dyn_cast<DbgValueInst>(&I)) {
          insertVarName(dvi);
        }
      }
    }
  }

  Units& units;

public:
  VarNameParser(Units& units_) : units(units_) {
    for (auto* function : units.functions.getAnalyzedFunctions()) {
      units.setActiveFunction(function);
      insertVarNames(function);
    }
  }
};

} // namespace llvm
