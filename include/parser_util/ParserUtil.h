#pragma once
#include "Common.h"
#include "analysis_util/MemoryUtil.h"

namespace llvm {

template <typename Units>
auto* getDILocalVar(Units& units, Instruction* instr) {
  // try objects
  if (units.variables.instrHasLocalVar(instr)) {
    auto* var = units.variables.getLocalVar(instr);
    return var;
  }

  if (auto* gepi = getGepi(instr)){
      auto* var = units.variables.getLocalVar(gepi);
      return var;
  }

  return (DILocalVariable*)nullptr;
}

} // namespace llvm
