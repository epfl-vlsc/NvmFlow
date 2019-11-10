#pragma once
#include "Common.h"

#include "ds/InstrInfo.h"
#include "parser_util/AliasGroups.h"
#include "parser_util/InstrParser.h"

namespace llvm {

template <typename Globals> class VarParser {
  template <typename AliasInfo>
  void addInstrInfo(FunctionSet& funcSet, AliasInfo& ai) {
    std::set<std::pair<StructType*, int>> seenSts;
    for (auto* f : funcSet) {
      for (auto& I : instructions(*f)) {
        // get instruction type
        auto instrType = InstrInfo::getInstrType(&I, globals);

        if (!InstrInfo::isUsedInstr(instrType))
          continue;

        // check non variable based parsing
        if (InstrInfo::isNonVarInstr(instrType)) {
          auto pv = ParsedVariable();
          globals.locals.addInstrInfo(&I, instrType, nullptr, pv);
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
        auto* alias = pv.getObjAlias();
        int setNo = ai.getSetNo(alias);
        // add st variable
        if (!seenSts.count({st, setNo})) {
          var = globals.locals.addVariable(st, setNo);
          seenSts.insert({st, setNo});
        } else {
          var = globals.locals.getVariable(st, setNo);
        }

        // sf info
        if (pv.isField()) {
          // field
          auto [st, idx] = pv.getStructInfo();
          auto* sf = globals.dbgInfo.getStructField(st, idx);
          if (!sf)
            continue;

          var = globals.locals.addVariable(sf, setNo);
        }

        globals.locals.addInstrInfo(&I, instrType, var, pv);
      }
    }
  }

  template <typename AliasInfo>
  void createAliasSets(FunctionSet& funcSet, AliasInfo& ai) {
    for (auto* f : funcSet) {
      for (auto& I : instructions(*f)) {
        // get instruction type
        auto instrType = InstrInfo::getInstrType(&I, globals);
        if (!InstrInfo::isVarInstr(instrType))
          continue;

        // lhs-----------------------------------
        auto pv = InstrParser::parseVarLhs(&I);
        if (!pv.isUsed() || !pv.isPersistentVar())
          continue;

        if (pv.isField()) {
          auto [st, idx] = pv.getStructInfo();

          if (!globals.dbgInfo.isUsedStructType(st))
            continue;
        }

        auto* alias = pv.getObjAlias();
        ai.insert(alias);
      }
    }
  }

  void addVars() {
    for (auto* f : globals.functions.getAnalyzedFunctions()) {
      globals.setActiveFunction(f);
      auto funcSet = globals.functions.getUnitFunctionSet(f);
      auto& AAR = globals.getAliasAnalysis();

      AliasGroups ag(AAR);
      createAliasSets(funcSet, ag);
      addInstrInfo(funcSet, ag);
    }
  }

  Globals& globals;

public:
  VarParser(Globals& globals_) : globals(globals_) { addVars(); }
};

} // namespace llvm