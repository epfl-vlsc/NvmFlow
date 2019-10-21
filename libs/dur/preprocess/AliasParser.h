#pragma once
#include "Common.h"

#include "ds/InstrInfo.h"
#include "parser_util/AliasGroups.h"
#include "parser_util/InstrParser.h"

namespace llvm {

template <typename Globals> class AliasParser {
  static constexpr const char* DurableField = "DurableField";

  bool isTrackedVar(ParsedVariable& pv) {
    if (!pv.isUsed())
      return false;

    StructField* sf = nullptr;
    if (pv.isField()) {
      auto [st, idx] = pv.getStructInfo();

      if (!globals.dbgInfo.isUsedStructType(st))
        return false;

      sf = globals.dbgInfo.getStructField(st, idx);
    }

    // field not tracked and type not tracked
    auto* type = pv.getType();
    if (!sf && !globals.dbgInfo.isTrackedType(type))
      return false;

    return true;
  }

  auto getParsedVarRhs(Instruction* i, bool annotated) {
    if (annotated)
      return InstrParser::parseVarRhs(i);
    return InstrParser::parseEmpty();
  }

  bool isAnnotated(ParsedVariable& pv) const {
    return pv.isAnnotated() && pv.getAnnotation().equals(DurableField);
  }

  Variable* getVariableRhs(ParsedVariable& pvRhs, AliasGroups& ag) {
    if (!ag.isValidAlias(pvRhs)) {
      return nullptr;
    }

    int rhsSetNo = ag.getAliasSetNo(pvRhs);
    auto* rhs = globals.locals.getVariable(rhsSetNo);
    return rhs;
  }

  void addInstrInfo(FunctionSet& funcSet, AliasGroups& ag) {
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
        if (!isTrackedVar(pv))
          continue;

        // lhs---------------------------------------------
        int lhsSetNo = ag.getAliasSetNo(pv);
        auto* lhs = globals.locals.getVariable(lhsSetNo);

        // rhs--------------------------------------------
        auto pvRhs = getParsedVarRhs(&I, isAnnotated(pv));
        auto* rhs = getVariableRhs(pvRhs, ag);

        // add var based instruction
        globals.locals.addInstrInfo(&I, instrType, lhs, rhs, pv, pvRhs);
      }
    }
  }

  void createAliasSets(FunctionSet& funcSet, AliasGroups& ag) {
    for (auto* f : funcSet) {
      for (auto& I : instructions(*f)) {
        // get instruction type
        auto instrType = InstrInfo::getInstrType(&I, globals);
        if (!InstrInfo::isVarInstr(instrType))
          continue;

        // lhs-----------------------------------
        auto pv = InstrParser::parseVarLhs(&I);
        if (!isTrackedVar(pv))
          continue;

        ag.insert(pv);

        // rhs-------------------------------------
        bool isAnnot = isAnnotated(pv);
        if (!isAnnot)
          continue;

        auto pvRhs = getParsedVarRhs(&I, isAnnot);
        if (!pvRhs.isUsed())
          continue;
        ag.insert(pvRhs);
      }
    }

    // create lattice variables
    for (int i = 0; i < ag.size(); ++i) {
      globals.locals.addVariable(i);
    }
  }

  void addLocals() {
    for (auto* f : globals.functions.getAnalyzedFunctions()) {
      globals.setActiveFunction(f);
      auto funcSet = globals.functions.getUnitFunctionSet(f);
      auto& AAR = globals.getAliasAnalysis();

      AliasGroups ag(AAR);
      createAliasSets(funcSet, ag);
      // addInstrInfo(funcSet, ag);
    }
  }

  Globals& globals;

public:
  AliasParser(Globals& globals_) : globals(globals_) { addLocals(); }
};

} // namespace llvm