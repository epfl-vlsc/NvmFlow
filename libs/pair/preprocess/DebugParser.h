#pragma once
#include "Common.h"

#include "AnnotParser.h"
#include "parser_util/InstrParser.h"
#include "ds/Globals.h"

namespace llvm {

class DebugParser {
  void checkVar(Instruction* i) {
    auto pv = InstrParser::parseInstruction(i);
    if (!pv.isAnnotated())
      return;

    auto annotation = pv.getAnnotation();
    if (AnnotParser::isValidAnnotation(annotation)) {
      auto* st = pv.getStructType();
      structTypes.insert(st);
    }
  }

  void checkRead(LoadInst* li) { checkVar(li); }

  void checkWrite(StoreInst* si) { checkVar(si); }

  void checkFlush(CallInst* ci) { checkVar(ci); }

  void checkCall(CallInst* ci) {
    auto* f = ci->getCalledFunction();
    if (globals.functions.isSkippedFunction(f)) {
      return;
    } else if (globals.functions.isFlushFunction(f)) {
      checkFlush(ci);
    } else if (globals.functions.isFlushFenceFunction(f)) {
      checkFlush(ci);
    }
  }

  void addToStructTypes() {
    for (auto* f : globals.functions.getAllAnalyzedFunctions()) {
      for (auto& I : instructions(*f)) {
        if (auto* si = dyn_cast<StoreInst>(&I)) {
          checkWrite(si);
        } else if (auto* li = dyn_cast<LoadInst>(&I)) {
          // checkRead(li);
        } else if (auto* ci = dyn_cast<CallInst>(&I)) {
          checkCall(ci);
        }
      }
    }
  }

  void addDbgInfoFunctions() {
    auto& funcSet = globals.functions.getAllAnalyzedFunctions();
    globals.dbgInfo.addDbgInfoFunctions(funcSet, structTypes);
  }

  Globals& globals;
  std::set<StructType*> structTypes;

public:
  DebugParser(Globals& globals_) : globals(globals_) {
    addToStructTypes();
    addDbgInfoFunctions();
  }
}; // namespace llvm

} // namespace llvm