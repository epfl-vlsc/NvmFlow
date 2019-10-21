#pragma once
#include "Common.h"

#include "ds/InstrInfo.h"
#include "parser_util/InstrParser.h"

namespace llvm {

template <typename Globals> class DbgParser {
  static constexpr const char* DurableField = "DurableField";

  void addAnnotatedType(ParsedVariable& pv, std::set<Type*>& ptrTypes,
                        std::set<StructType*>& structTypes) {
    // check annotation type
    auto annotation = pv.getAnnotation();
    if (!annotation.equals(DurableField))
      return;

    // get ptr type
    auto* ptrType = pv.getType();
    assert(ptrType->isPointerTy());
    ptrTypes.insert(ptrType);

    // get struct type
    auto* structType = pv.getStructType();
    structTypes.insert(structType);
  }

  void addFlushedType(ParsedVariable& pv, std::set<Type*>& ptrTypes,
                      std::set<StructType*>& structTypes) {
    auto* ptrType = pv.getType();
    assert(ptrType->isPointerTy());
    ptrTypes.insert(ptrType);
  }

  void addUsedTypes(Function* func, std::set<Type*>& ptrTypes,
                    std::set<StructType*>& structTypes) {
    for (auto* f : globals.functions.getUnitFunctions(func)) {
      for (auto& I : instructions(*f)) {
        // parse instr
        auto pv = InstrParser::parseVarLhs(&I);
        if (!pv.isUsed())
          continue;

        // ordering here matters
        if (pv.isAnnotated())
          addAnnotatedType(pv, ptrTypes, structTypes);
        else if (globals.functions.isAnyFlushFunction(&I))
          addFlushedType(pv, ptrTypes, structTypes);
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