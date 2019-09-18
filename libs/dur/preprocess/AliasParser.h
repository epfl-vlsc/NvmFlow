#pragma once
#include "Common.h"

#include "analysis_util/AliasGroups.h"
#include "ds/InstrInfo.h"
#include "parser_util/InstrParser.h"

namespace llvm {

template <typename Globals> class AliasParser {
  using InstrType = typename InstrInfo::InstrType;

  static constexpr const char* DurableField = "DurableField";

  struct LocRefData {
    Instruction* instr;
    InstrType instrType;
    Variable var;
    Value* rhsAlias;
  };

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
    } else {
      return InstrType::IpInstr;
    }
  }

  auto getInstrType(Instruction* i) const {
    if (auto* si = dyn_cast<StoreInst>(i)) {
      return InstrType::WriteInstr;
    } else if (auto* ci = dyn_cast<CallInst>(i)) {
      return getCallInstrType(ci);
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
          globals.locals.addInstrInfo(&I, instrType, nullptr, nullptr);
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
        bool annotated =
            pv.isAnnotated() && pv.getAnnotation().equals(DurableField);

        if (pv.isField()) {
          auto [st, idx] = pv.getStructInfo();

          if (!globals.dbgInfo.isUsedStructType(st))
            continue;

          sf = globals.dbgInfo.getStructField(st, idx);
        }

        // find local name
        auto* lv = pv.getLocalVar();
        // todo need to handle phi
        auto* diVar = globals.dbgInfo.getDILocalVariable(lv);
        std::string localName = (diVar) ? diVar->getName().str() : "";

        auto var = VarInfo::getVarInfo(pv, sf, annotated, localName);
        auto* rhs = pv.getRhs();

        // create alias sets
        auto* lhs = pv.getOpndVar();
        ag.insert(lhs);
        if (rhs) {
          ag.insert(rhs);
        }

        // only var references in this loop
        if (pv.isLocRef()) {
          // location references
          locReferences.push_back({&I, instrType, var, rhs});
        } else {
          // var references
          auto* varPtr = globals.locals.addVariable(var);
          globals.locals.addInstrInfo(&I, instrType, varPtr, rhs);
        }
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
        auto* rhsAlias = pv.getRhs();
        ag.insert(lhsAlias);
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
      ag.print(errs());

      //addInstrInfo(funcSet, ag);
    }
  }

  Globals& globals;
  std::vector<LocRefData> locReferences;

public:
  AliasParser(Globals& globals_) : globals(globals_) { addLocals(); }
};

} // namespace llvm