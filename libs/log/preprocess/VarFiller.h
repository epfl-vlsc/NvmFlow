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
      auto* obj = globals.locals.getVariable(st);
      var->addToWriteSet(obj);
    }
  }

  void fillFlushSets() {
    for (auto& variable : globals.locals.getVariables()) {
      auto* var = (Variable*)&variable;

      if (var->isField())
        continue;

      auto* st = var->getStructType();
      for (auto* sf : globals.dbgInfo.getFieldMap(st)) {
        if (!globals.locals.inVariables(sf))
          continue;

        auto* field = globals.locals.getVariable(sf);
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