#pragma once
#include "Common.h"

#include "analysis_util/AliasGroups.h"
#include "ds/InstrInfo.h"
#include "parser_util/InstrParser.h"

namespace llvm {

template <typename Globals> class AliasParser {
  using InstrType = typename InstrInfo::InstrType;

  static constexpr const char* DurableField = "DurableField";

  auto getCallInstrType(CallInst* ci) const {
    auto* callee = ci->getCalledFunction();

    if (globals.functions.isSkippedFunction(callee)) {
      return InstrType::None;
    } else if (globals.functions.isPfenceFunction(callee)) {
      return InstrType::PfenceInstr;
    } else if (globals.functions.isVfenceFunction(callee)) {
      return InstrType::VfenceInstr;
    } else if (globals.functions.isFlushFunction(callee)) {
      return InstrType::FlushInstr;
    } else if (globals.functions.isFlushFenceFunction(callee)) {
      return InstrType::FlushFenceInstr;
    } else if (globals.functions.isStoreFunction(callee)) {
      return InstrType::WriteInstr;
    } else {
      return InstrType::IpInstr;
    }
  }

  auto getInstrType(Instruction* i) const {
    if (auto* si = dyn_cast<StoreInst>(i)) {
      return InstrType::WriteInstr;
    } else if (auto* ci = dyn_cast<CallInst>(i)) {
      return getCallInstrType(ci);
    } else if (auto* ii = dyn_cast<InvokeInst>(i)) {
      return InstrType::IpInstr;
    }

    return InstrType::None;
  }

  void addInstrInfo(FunctionSet& funcSet, AliasGroups& ag) {
    for (auto* f : funcSet) {
      for (auto& I : instructions(*f)) {
        // get instruction type
        auto instrType = getInstrType(&I);
        if (!InstrInfo::isUsedInstr(instrType))
          continue;

        // check non variable based parsing
        if (InstrInfo::isNonVarInstr(instrType)) {
          globals.locals.addInstrInfo(&I, instrType, nullptr, nullptr, nullptr);
          continue;
        }

        // parse variable based
        auto pv = InstrParser::parseInstruction(&I);
        if (!pv.isUsed())
          continue;

        // check tracked types
        auto* type = pv.getType();
        if (!globals.dbgInfo.isTrackedType(type))
          continue;

        // variable info
        StructField* sf = nullptr;

        if (pv.isField()) {
          auto [st, idx] = pv.getStructInfo();

          if (!globals.dbgInfo.isUsedStructType(st))
            continue;

          sf = globals.dbgInfo.getStructField(st, idx);
        }

        // annotated
        bool annotated =
            pv.isAnnotated() && pv.getAnnotation().equals(DurableField);

        // find local name
        auto* lv = pv.getLocalVar();
        // todo need to handle phi
        auto* diVar = globals.dbgInfo.getDILocalVariable(lv);
        std::string localName = (diVar) ? diVar->getName().str() : "";

        auto* lhsAlias = pv.getOpndVar();
        auto* rhsAlias = pv.getRhs();

        auto var = VarInfo::getVarInfo(pv, sf, annotated, localName);

        // create var info
        auto* varInfo = globals.locals.addVarInfo(var);

        // add aliases
        Variable* rhsVar = nullptr;

        int lhsNo = ag.getAliasSetNo(lhsAlias);
        auto* lhsVar = globals.locals.getAliasSet(lhsNo);
        globals.locals.addAlias(lhsAlias, lhsVar);

        if (rhsAlias) {
          int rhsNo = ag.getAliasSetNo(rhsAlias);
          if (!AliasGroups::isInvalidNo(rhsNo)) {
            rhsVar = globals.locals.getAliasSet(rhsNo);
            globals.locals.addAlias(rhsAlias, rhsVar);
          }
        }

        // add var based instruction
        globals.locals.addInstrInfo(&I, instrType, varInfo, lhsVar, rhsVar);
      }
    }
  }

  void createAliasSets(FunctionSet& funcSet, AliasGroups& ag) {
    for (auto* f : funcSet) {
      for (auto& I : instructions(*f)) {
        // get instruction type
        auto instrType = getInstrType(&I);
        if (!InstrInfo::isUsedInstr(instrType))
          continue;

        // check non variable based parsing
        if (InstrInfo::isNonVarInstr(instrType)) {
          continue;
        }

        // parse variable based
        auto pv = InstrParser::parseInstruction(&I);
        if (!pv.isUsed())
          continue;

        // check tracked types
        auto* type = pv.getType();
        if (!globals.dbgInfo.isTrackedType(type))
          continue;

        if (pv.isField()) {
          auto [st, idx] = pv.getStructInfo();

          if (!globals.dbgInfo.isUsedStructType(st))
            continue;
        }

        // create alias sets
        auto* lhsAlias = pv.getOpndVar();

        if (pv.isObjPtr()) {
          auto* lv = pv.getLocalVar();
          ag.insert(lhsAlias, lv);
        } else {
          ag.insert(lhsAlias);
        }

        auto* rhsAlias = pv.getRhs();
        if (rhsAlias) {
          ag.insert(rhsAlias);
        }
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
      addInstrInfo(funcSet, ag);
    }
  }

  Globals& globals;

public:
  AliasParser(Globals& globals_) : globals(globals_) { addLocals(); }
};

} // namespace llvm