#pragma once
#include "Common.h"
#include "analysis_util/MemoryUtil.h"

namespace llvm {

template <typename Units> auto* getDILocalVar(Units& units, Value* v) {
  // try objects
  if (auto* var = units.variables.getLocalVar(v)) {
    return var;
  }

  // try field
  if (auto* gepi = getGepi(v)) {
    auto* var = units.variables.getLocalVar(gepi);
    return var;
  }

  return (DILocalVariable*)nullptr;
}

} // namespace llvm
