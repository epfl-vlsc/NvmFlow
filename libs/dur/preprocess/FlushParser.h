#pragma once
#include "Common.h"

#include "analysis_util/MemoryUtil.h"
#include "ds/Units.h"

namespace llvm {

class FlushParser {
  using InstructionType = typename InstructionInfo::InstructionType;

  void insertFlush(CallInst* ci, InstructionType instrType) {
    auto* arg0 = ci->getArgOperand(0);
    if (auto* arg0Instr = dyn_cast<Instruction>(arg0)) {
      auto accessInfo = AccessType::getAccessInfo(arg0Instr);
      errs() << accessInfo.getName() << "\n";
    }
  }

  void insertCall(CallInst* ci) {
    auto* callee = ci->getCalledFunction();

    if (units.functions.isFlushFunction(callee)) {
      insertFlush(ci, InstructionInfo::FlushInstr);
    } else if (units.functions.isFlushFenceFunction(callee)) {
      insertFlush(ci, InstructionInfo::FlushFenceInstr);
    }
  }

  void insertI(Instruction* i) {
    if (auto* ci = dyn_cast<CallInst>(i)) {
      insertCall(ci);
    }
  }

  void initFlushedVariables(Function* function) {
    for (auto* f : units.functions.getUnitFunctions(function)) {
      for (auto& I : instructions(*f)) {
        insertI(&I);
      }
    }
  }

  void initAliasSetTracker(Function* function) {
    auto& AAR = units.getAliasAnalysisResults();
    auto* ast = new AliasSetTracker(AAR);
    for (auto* f : units.functions.getUnitFunctions(function)) {
      for (auto& BB : *f) {
        ast->add(BB);
      }
    }
    units.variables.setAliasSetTracker(ast);
  }

  Units& units;

public:
  FlushParser(Units& units_) : units(units_) {
    for (auto* function : units.functions.getAnalyzedFunctions()) {
      units.setActiveFunction(function);
      initAliasSetTracker(function);
      initFlushedVariables(function);
    }
  }
};

} // namespace llvm