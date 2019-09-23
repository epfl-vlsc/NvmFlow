#pragma once
#include "Common.h"

#include "AnnotParser.h"
#include "parser_util/InstrParser.h"

namespace llvm {

template <typename Globals> class DbgParser {
  void addUsedTypes(Function* func, std::set<Type*>& ptrTypes,
                    std::set<StructType*>& structTypes) {
    for (auto* f : globals.functions.getUnitFunctions(func)) {
      for (auto& I : instructions(*f)) {
        // parse instr
        auto pv = InstrParser::parseInstruction(&I);
        if (!pv.isUsed() || !pv.isAnnotated())
          continue;

        // check annotation type
        auto annotation = pv.getAnnotation();
        if (AnnotParser::isValidAnnotation(annotation)) {
          auto* st = pv.getStructType();
          structTypes.insert(st);
        }
      }
    }
  }

  void addAllUsedTypes() {
    std::set<Type*> ptrTypes;
    std::set<StructType*> structTypes;

    for (auto* f : globals.functions.getAnalyzedFunctions()) {
      globals.setActiveFunction(f);
      addUsedTypes(f, ptrTypes, structTypes);
    }

    // add to global dbg info
    auto& funcSet = globals.functions.getAllAnalyzedFunctions();
    globals.dbgInfo.addDbgInfoFunctions(funcSet, ptrTypes, structTypes);
  }

  Globals& globals;

public:
  DbgParser(Globals& globals_) : globals(globals_) { addAllUsedTypes(); }
};

} // namespace llvm