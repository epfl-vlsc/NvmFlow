#pragma once
#include "Common.h"

#include "ds/InstrInfo.h"
#include "parser_util/InstrParser.h"

namespace llvm {

template <typename Globals> class VarParser {
  using InstrType = typename InstrInfo::InstrType;

  auto getCallInstrType(CallInst* ci) const {
    auto* callee = ci->getCalledFunction();

    if (globals.functions.isLoggingFunction(callee)) {
      return InstrType::LoggingInstr;
    } else if (globals.functions.isTxBeginFunction(callee)) {
      return InstrType::TxBegInstr;
    } else if (globals.functions.isTxEndFunction(callee)) {
      return InstrType::TxEndInstr;
    } else if (globals.functions.isStoreFunction(callee)) {
      return InstrType::WriteInstr;
    } else if (globals.functions.isSkippedFunction(callee)) {
      return InstrType::None;
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

  void addInstrInfo(Function* func) {
    std::set<StructType*> seenSts;
    for (auto* f : globals.functions.getUnitFunctions(func)) {
      for (auto& I : instructions(*f)) {
        // get instruction type
        auto instrType = getInstrType(&I);

        if (!InstrInfo::isUsedInstr(instrType))
          continue;

        // check non variable based parsing
        if (InstrInfo::isNonVarInstr(instrType)) {
          globals.locals.addInstrInfo(&I, instrType, nullptr, ParsedVariable());
          continue;
        }

        // parse variable based
        auto pv = InstrParser::parseInstruction(&I);
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