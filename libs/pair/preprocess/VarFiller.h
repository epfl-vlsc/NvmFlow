#pragma once
#include "Common.h"

namespace llvm {

template <typename Globals> class VarFiller {
  void fillWriteSets() {
    for (auto& variable : globals.locals.getVariables()) {
      auto* var = (Variable*)&variable;

      if (globals.locals.inSentinels(var) || var->isObj())
        continue;

      auto* st = var->getStructType();
      auto* obj = globals.locals.getVariable(st);
      var->addToWriteSet(obj);
    }
  }

  void fillFlushSets() {
    for (auto& variable : globals.locals.getVariables()) {
      auto* var = (Variable*)&variable;

      // assuming: sentinels dont have flush sets
      if (globals.locals.inSentinels(var) || !var->isObj())
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

  void fillPairs() {
    for (auto& pairVar : globals.locals.getPairs()) {
      auto* pair = (PairVariable*)&pairVar;
      auto [data, valid] = pair->getPair();
      data->addToPairs(pair);
      valid->addToPairs(pair);
    }
  }

  Globals& globals;

public:
  VarFiller(Globals& globals_) : globals(globals_) {
    for (auto* function : globals.functions.getAnalyzedFunctions()) {
      globals.setActiveFunction(function);
      fillWriteSets();
      fillFlushSets();
      fillPairs();
    }
  }
};

} // namespace llvm