#pragma once
#include "Common.h"

namespace llvm {

template <typename Globals> class VarFiller {
  void fillWriteSets() {
    for (auto& variable : globals.locals.getVariables()) {
      auto* var = (Variable*)&variable;

      if (var->isObj())
        continue;

      auto* st = var->getStructType();
      int setNo = var->getSetNo();
      auto* obj = globals.locals.getVariable(st, setNo);
      var->addToWriteSet(obj);
    }
  }

  void fillFlushSets() {
    for (auto& variable : globals.locals.getVariables()) {
      auto* var = (Variable*)&variable;

      if (var->isField())
        continue;

      auto* st = var->getStructType();
      int setNo = var->getSetNo();
      for (auto* sf : globals.dbgInfo.getFieldMap(st)) {
        if (!globals.locals.inVariables(sf, setNo))
          continue;

        auto* field = globals.locals.getVariable(sf, setNo);
        var->addToFlushSet(field);
      }
    }
  }

  Globals& globals;

public:
  VarFiller(Globals& globals_) : globals(globals_) {
    for (auto* function : globals.functions.getAnalyzedFunctions()) {
      globals.setActiveFunction(function);
      fillWriteSets();
      fillFlushSets();
    }
  }
};

} // namespace llvm