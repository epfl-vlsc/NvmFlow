#pragma once
#include "Common.h"

#include "analysis_util/MemoryUtil.h"
#include "ds/Units.h"
#include "parser_util/ParserUtil.h"

namespace llvm {

class AliasParser {
  void insertWrite(StoreInst* si) { units.variables.insertAliasInstr(si); }

  void insertFlush(CallInst* ci) { units.variables.insertAliasInstr(ci); }

  void insertCall(CallInst* ci) {
    auto* callee = ci->getCalledFunction();

    if (!callee || callee->isIntrinsic() ||
        units.functions.isSkippedFunction(callee)) {
      return;
    } else if (units.functions.isFlushFunction(callee)) {
      insertFlush(ci);
    } else if (units.functions.isFlushFenceFunction(callee)) {
      insertFlush(ci);
    }
  }

  void insertI(Instruction* i) {
    if (auto* si = dyn_cast<StoreInst>(i)) {
      insertWrite(si);
    } else if (auto* ci = dyn_cast<CallInst>(i)) {
      insertCall(ci);
    }
  }

  void insertPointers(Function* function) {
    for (auto* f : units.functions.getUnitFunctions(function)) {
      for (auto& I : instructions(*f)) {
        insertI(&I);
      }
    }
  }

  Units& units;

public:
  AliasParser(Units& units_) : units(units_) {
    for (auto* function : units.functions.getAnalyzedFunctions()) {
      units.setActiveFunction(function);
      insertPointers(function);
      units.createAliasGroups();
    }
  }
};

} // namespace llvm