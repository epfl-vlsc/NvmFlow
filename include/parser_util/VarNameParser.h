#pragma once
#include "Common.h"

namespace llvm {

template <typename Units> class VarNameParser {
  std::set<Value*> visited;

  void insertVarUsers(Value* val, DILocalVariable* var) {
    if (visited.count(val))
      return;

    visited.insert(val);

    if (auto* ci = dyn_cast<CastInst>(val)) {
      auto* v = ci->stripPointerCasts();
      insertVarUsers(v, var);
    }

    units.variables.insertLocalVariable(val, var);

    for (auto* u : val->users()) {
      if (Instruction* i = dyn_cast<Instruction>(u)) {
        insertVarUsers(i, var);
      }
    }
  }

  void insertVarName(DbgValueInst* dvi) {
    auto* val = dvi->getValue();
    auto* var = dvi->getVariable();
    assert(var && val);
    insertVarUsers(val, var);
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
