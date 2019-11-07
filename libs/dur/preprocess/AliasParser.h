#pragma once
#include "Common.h"

#include "ds/InstrInfo.h"
#include "parser_util/AliasGroups.h"
#include "parser_util/InstrParser.h"

namespace llvm {

template <typename Globals> class AliasParser {
  static constexpr const char* DurableField = "DurableField";

  bool isSkipPv(ParsedVariable& pv) {
    if (!pv.isUsed())
      return true;

    if (pv.isField()) {
      auto [st, idx] = pv.getStructInfo();

      return !globals.dbgInfo.isUsedStructType(st);
    }

    auto* type = pv.getType();
    return !globals.dbgInfo.isTrackedType(type);
  }

  auto getParsedVarRhs(Instruction* i, bool annotated) {
    if (annotated)
      return InstrParser::parseVarRhs(i);
    return InstrParser::parseEmpty();
  }

  bool isAnnotated(ParsedVariable& pv) const {
    return pv.isAnnotated() && pv.getAnnotation().equals(DurableField);
  }

  template<typename AliasInfo>
  Variable* getVariableRhs(ParsedVariable& pvRhs, AliasInfo& ai) {
    auto* aliasRhs = pvRhs.getAlias();
    int rhsSetNo = ai.getSetNo(aliasRhs);
    if (!ai.isValidSet(rhsSetNo)) {
      return nullptr;
    }

    auto* rhs = globals.locals.getVariable(rhsSetNo);
    return rhs;
  }

  template<typename AliasInfo>
  void addInstrInfo(FunctionSet& funcSet, AliasInfo& ai) {
    for (auto* f : funcSet) {
      for (auto& I : instructions(*f)) {
        // get instruction type
        auto instrType = InstrInfo::getInstrType(&I, globals);
        if (!InstrInfo::isUsedInstr(instrType))
          continue;

        // check non variable based parsing
        if (InstrInfo::isNonVarInstr(instrType)) {
          auto pv = ParsedVariable();
          auto pvRhs = ParsedVariable(false);
          globals.locals.addInstrInfo(&I, instrType, nullptr, nullptr, pv,
                                      pvRhs);
          continue;
        }

        // parse variable based
        auto pv = InstrParser::parseVarLhs(&I);

        if (isSkipPv(pv))
          continue;

        // lhs---------------------------------------------
        auto* aliasLhs = pv.getAlias();
        int lhsSetNo = ai.getSetNo(aliasLhs);
        auto* lhs = globals.locals.getVariable(lhsSetNo);

        // rhs--------------------------------------------
        auto pvRhs = getParsedVarRhs(&I, isAnnotated(pv));
        auto* rhs = getVariableRhs(pvRhs, ai);

        // add var based instruction
        globals.locals.addInstrInfo(&I, instrType, lhs, rhs, pv, pvRhs);
      }
    }
  }

  template<typename AliasInfo>
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

        auto* aliasLhs = pv.getAlias();
        ai.insert(aliasLhs);

        // rhs-------------------------------------
        bool isAnnot = isAnnotated(pv);
        if (!isAnnot)
          continue;

        auto pvRhs = getParsedVarRhs(&I, isAnnot);
        if (!pvRhs.isUsed())
          continue;
        auto* aliasRhs = pvRhs.getAlias();
        ai.insert(aliasRhs);
      }
    }

    // create lattice variables
    for (int i = 0; i < ai.size(); ++i) {
      globals.locals.addVariable(i);
    }
  }

  void addLocals() {
    for (auto* f : globals.functions.getAnalyzedFunctions()) {
      globals.setActiveFunction(f);
      auto funcSet = globals.functions.getUnitFunctionSet(f);
      auto& AAR = globals.getAliasAnalysis();

      SparseAliasGroups ag(AAR);
      createAliasSets(funcSet, ag);
      addInstrInfo(funcSet, ag);
    }
  }

  Globals& globals;

public:
  AliasParser(Globals& globals_) : globals(globals_) { addLocals(); }
};

} // namespace llvm