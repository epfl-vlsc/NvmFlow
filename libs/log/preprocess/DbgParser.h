#pragma once
#include "Common.h"

#include "parser_util/InstrParser.h"

namespace llvm {

template <typename Globals> class DbgParser {
  static constexpr const char* PersistentName = "_ZL14pmemobj_direct7pmemoid";

  bool isPersistentVar(Value* v) const {
    if (auto* ci = dyn_cast<CallInst>(v)) {
      auto* f = ci->getCalledFunction();
      if (f->getName().equals(PersistentName))
        return true;
    }
    return false;
  }

  void addUsedTypes(Function* func, std::set<Type*>& ptrTypes,
                    std::set<StructType*>& structTypes) {
    for (auto* f : globals.functions.getUnitFunctions(func)) {
      for (auto& I : instructions(*f)) {
        // parse instr
        auto pv = InstrParser::parseInstruction(&I);
        if (!pv.isUsed())
          continue;

        // check annotation type
        auto* lv = pv.getLocalVar();
        if (!isPersistentVar(lv))
          continue;

        if (auto* persistentSt = pv.getObjStructType())
          structTypes.insert(persistentSt);
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