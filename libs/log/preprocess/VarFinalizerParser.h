#pragma once
#include "Common.h"

#include "analysis_util/MemoryUtil.h"
#include "ds/Units.h"

namespace llvm {

class VarFinalizerParser {
  void fillFlushField() {
    for (auto* var : units.variables.getVariables()) {
      if (!var->isObj())
        continue;

      assert(var->isObj());
      for (auto* field : units.dbgInfo.getFieldMap(var)) {
        if (!units.variables.inVars(field))
          continue;
        units.variables.insertFlushField(var, field);
      }
    }
  }

  void fillObjFieldInfo() { fillFlushField(); }

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