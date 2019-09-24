#pragma once
#include "Common.h"

#include "ds/InstrInfo.h"
#include "parser_util/InstrParser.h"

namespace llvm {

template <typename Globals> class VarParser {
  /*
using InstrType = typename InstrInfo::InstrType;

void addVar(Instruction* i, InstrType instrType) {
  auto pv = InstrParser::parseInstruction(i);
  if (!pv.isUsed())
    return;

  if (!pv.isAnnotated())
    return;

  // get annotation
  auto annot = pv.getAnnotation();
  if (!AnnotParser::isValidAnnotation(annot))
    return;

  // valid
  assert(pv.isField());
  auto [st, idx] = pv.getStructInfo();
  auto* validSf = globals.dbgInfo.getStructField(st, idx);
  auto* valid = globals.locals.addVariable(validSf);
  globals.locals.addSentinel(valid);

  // obj
  auto* obj = globals.locals.addVariable(st);

  // data
  auto [parsedAnnot, useDcl] = AnnotParser::parseAnnotation(annot);
  auto* data = (Variable*)nullptr;
  if (!parsedAnnot.empty()) {
    // data
    auto* dataSf = globals.dbgInfo.getStructField(parsedAnnot);
    data = globals.locals.addVariable(dataSf);
  } else {
    // obj
    data = obj;
  }

  // pair
  globals.locals.addPair(data, valid, useDcl);

  // ii
  globals.locals.addInstrInfo(i, instrType, valid, pv);
}

void addWrite(StoreInst* si) { addVar(si, InstrType::WriteInstr); }

void addFlush(CallInst* ci, InstrType instrType) { addVar(ci, instrType); }

void addInstrInfo(CallInst* ci, InstrType instrType) {
  auto pv = ParsedVariable();
  globals.locals.addInstrInfo(ci, instrType, nullptr, pv);
}

void addCall(CallInst* ci) {
  auto* callee = ci->getCalledFunction();

  if (globals.functions.isSkippedFunction(callee)) {
    return;
  } else if (globals.functions.isPfenceFunction(callee)) {
    addInstrInfo(ci, InstrInfo::PfenceInstr);
  } else if (globals.functions.isVfenceFunction(callee)) {
    addInstrInfo(ci, InstrInfo::VfenceInstr);
  } else if (globals.functions.isFlushFunction(callee)) {
    addFlush(ci, InstrInfo::FlushInstr);
  } else if (globals.functions.isFlushFenceFunction(callee)) {
    addFlush(ci, InstrInfo::FlushFenceInstr);
  } else {
    addInstrInfo(ci, InstrInfo::IpInstr);
  }
}

void addI(Instruction* i) {
  if (auto* si = dyn_cast<StoreInst>(i)) {
    addWrite(si);
  } else if (auto* ci = dyn_cast<CallInst>(i)) {
    addCall(ci);
  }
}

  */

/*
  using InstrType = typename InstrInfo::InstrType;

  static constexpr const char* DurableField = "DurableField";

  auto getCallInstrType(CallInst* ci) const {
    auto* callee = ci->getCalledFunction();

    if (globals.functions.isSkippedFunction(callee)) {
      return InstrType::None;
    } else if (globals.functions.isLoggingFunction(callee)) {
      return InstrType::PfenceInstr;
    } else if (globals.functions.isTxBeginFunction(callee)) {
      return InstrType::PfenceInstr;
    } else if (globals.functions.isTxBeginFunction(callee)) {
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

  void addInstrInfo(FunctionSet& funcSet) {
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
*/
  void addVars(Function* func) {
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