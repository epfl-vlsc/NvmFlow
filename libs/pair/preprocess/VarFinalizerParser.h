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
    for (auto* var : units.variables.getDataSet()) {
      if (!var->isField())
        continue;

      auto* obj = units.dbgInfo.getStructObj(var);
      units.variables.insertWriteObj(var, obj);
    }
  }

  void fillWriteObjValid() {
    for (auto* var : units.variables.getValidSet()) {
      assert(var->isField());
      units.variables.insertWriteObj(var, nullptr);
    }
  }

  void fillObjFieldInfo() {
    fillWriteObjValid();
    fillWriteObjData();
    fillFlushFieldData();
  }

  Units& units;

public:
  VarFinalizerParser(Units& units_) : units(units_) { fillObjFieldInfo(); }
};

} // namespace llvm