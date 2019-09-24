#pragma once
#include "Common.h"

#include "parser_util/InstrParser.h"

namespace llvm {

template <typename Globals> class DbgParser {
  static constexpr const char* skipNames[] = {"_ZL14pmemobj_direct7pmemoid",
                                                  "pmemobj_pool_by_oid"};
  
  bool isASkipFunction(Function* f){
      for(auto * skipName : skipNames){
          if(f->getName().equals(skipName))
            globals.functions.insertSkipFunction(f);
      }
  }

  void addUsedTypes(Function* func, std::set<Type*>& ptrTypes,
                    std::set<StructType*>& structTypes) {
    for (auto* f : globals.functions.getUnitFunctions(func)) {
      errs() << f->getName() << "\n";
      /*
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
    */
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

  void addSkipFunctions() {}

  Globals& globals;

public:
  DbgParser(Globals& globals_) : globals(globals_) { addAllUsedTypes(); }
};

} // namespace llvm