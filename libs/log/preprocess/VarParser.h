#pragma once
#include "Common.h"

#include "ds/InstrInfo.h"
#include "parser_util/InstrParser.h"

namespace llvm {

template <typename Globals> class VarParser {
  void addInstrInfo(Function* func) {
    std::set<StructType*> seenSts;
    for (auto* f : globals.functions.getUnitFunctions(func)) {
      for (auto& I : instructions(*f)) {
        // get instruction type
        auto instrType = InstrInfo::getInstrType(&I, globals);

        if (!InstrInfo::isUsedInstr(instrType))
          continue;

        // check non variable based parsing
        if (InstrInfo::isNonVarInstr(instrType)) {
          globals.locals.addInstrInfo(&I, instrType, nullptr, ParsedVariable());
          continue;
        }

        // parse variable based
        auto pv = InstrParser::parseVarLhs(&I);
        if (!pv.isUsed() || !pv.isPersistentVar())
          continue;

        // check tracked types
        auto* st = pv.getObjStructType();
        if (!st || !globals.dbgInfo.isUsedStructType(st))
          continue;

        Variable* var = nullptr;
        // add st variable
        if (!seenSts.count(st)) {
          var = globals.locals.addVariable(st);
          seenSts.insert(st);
        } else {
          var = globals.locals.getVariable(st);
        }

        // sf info
        if (pv.isField()) {
          // field
          auto [st, idx] = pv.getStructInfo();
          auto* sf = globals.dbgInfo.getStructField(st, idx);
          var = globals.locals.addVariable(sf);
        }

        globals.locals.addInstrInfo(&I, instrType, var, pv);
      }
    }
  }

  void addVars() {
    for (auto* f : globals.functions.getAnalyzedFunctions()) {
      globals.setActiveFunction(f);
      addInstrInfo(f);
    }
  }

  Globals& globals;

public:
  VarParser(Globals& globals_) : globals(globals_) { addVars(); }
};

} // namespace llvm