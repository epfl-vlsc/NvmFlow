#pragma once
#include "Common.h"
#include "ds/Globals.h"

namespace llvm {

class VarFiller {
  /*
    void fillFlushFieldData() {
      for (auto* var : globals.variables.getDataSet()) {
        if (!var->isObj())
          continue;

        assert(var->isObj());
        for (auto* field : globals.dbgInfo.getFieldMap(var)) {
          if (globals.variables.inVars(field)) {
            globals.variables.insertFlushField(var, field);
          }
        }
      }
    }

    void fillWriteObjData() {
      for (auto* field : globals.variables.getDataSet()) {
        if (!field->isField() || globals.variables.inValidSet(field))
          continue;

        auto* obj = globals.dbgInfo.getStructObj(field);
        assert(obj);
        globals.variables.insertWriteObj(field, obj);
      }
    }

    void fillWriteObjValid() {
      for (auto* field : globals.variables.getValidSet()) {
        assert(field->isField());
        globals.variables.insertWriteObj(field, nullptr);
      }
    }
    */
  /*
    void fillObjFieldInfo() {
      fillWriteObjValid();
      fillWriteObjData();
      fillFlushFieldData();
    }
  */
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
      // fillFlushSet();
      // fillWriteSet();
      fillPairs();
    }
  }
};

} // namespace llvm