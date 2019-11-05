#pragma once
#include "Common.h"

#include "ds/InstrInfo.h"
#include "parser_util/AliasGroups.h"
#include "parser_util/InstrParser.h"
#include "ValidParser.h"
#include "DataParser.h"

namespace llvm {

template <typename Globals> class VarParser {
  bool isSkipPv(ParsedVariable& pv) {
    if (!pv.isUsed())
      return true;

    if (pv.isField()) {
      auto [st, idx] = pv.getStructInfo();

      return !globals.dbgInfo.isUsedStructType(st);
    }

    return false;
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
        if (isSkipPv(pv))
          continue;

        auto* aliasLhs = pv.getObjAlias();

        ai.insert(aliasLhs);
      }
    }
  }

  void addVars() {
    for (auto* f : globals.functions.getAnalyzedFunctions()) {
      globals.setActiveFunction(f);
      auto funcSet = globals.functions.getUnitFunctionSet(f);
      auto& AAR = globals.getAliasAnalysis();

      SparseAliasGroups ag(AAR);
      createAliasSets(funcSet, ag);
      ValidParser vParser(globals, funcSet, ag);
      DataParser dParser(globals, funcSet, ag);
    }
  }

  Globals& globals;

public:
  VarParser(Globals& globals_) : globals(globals_) { addVars(); }
};

} // namespace llvm