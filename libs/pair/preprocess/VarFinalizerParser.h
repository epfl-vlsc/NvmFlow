#pragma once
#include "Common.h"

#include "analysis_util/MemoryUtil.h"
#include "ds/Units.h"

namespace llvm {

class VarFinalizerParser {
  void fillFlushFieldData() {
    for (auto* var : units.variables.getDataSet()) {
      if (!var->isObj())
        continue;

      assert(var->isObj());
      for (auto* field : units.dbgInfo.getFieldMap(var)) {
        if (units.variables.inVars(field)) {
          units.variables.insertFlushField(var, field);
        }
      }
    }
  }

  void fillWriteObjData() {
    for (auto* field : units.variables.getDataSet()) {
      if (!field->isField() || units.variables.inValidSet(field))
        continue;

      auto* obj = units.dbgInfo.getStructObj(field);
      assert(obj);
      units.variables.insertWriteObj(field, obj);
    }
  }

  void fillWriteObjValid() {
    for (auto* field : units.variables.getValidSet()) {
      assert(field->isField());
      units.variables.insertWriteObj(field, nullptr);
    }
  }

  void fillObjFieldInfo() {
    fillWriteObjValid();
    fillWriteObjData();
    fillFlushFieldData();
  }

  Units& units;

public:
  VarFinalizerParser(Units& units_) : units(units_) {
    for (auto* function : units.functions.getAnalyzedFunctions()) {
      units.setActiveFunction(function);
      fillObjFieldInfo();
    }
  }
};

} // namespace llvm