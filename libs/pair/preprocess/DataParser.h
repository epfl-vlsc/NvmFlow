#pragma once
#include "Common.h"

#include "ds/InstrInfo.h"
#include "parser_util/InstrParser.h"

namespace llvm {

template <typename Globals> class DataParser {
  using InstrType = typename InstrInfo::InstrType;

  auto getCallInstrType(CallInst* ci) const {
    auto* callee = ci->getCalledFunction();

    if (globals.functions.isPfenceFunction(callee)) {
      return InstrType::PfenceInstr;
    } else if (globals.functions.isVfenceFunction(callee)) {
      return InstrType::VfenceInstr;
    } else if (globals.functions.isFlushFunction(callee)) {
      return InstrType::FlushInstr;
    } else if (globals.functions.isFlushFenceFunction(callee)) {
      return InstrType::FlushFenceInstr;
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

        // parse variable based
        auto pv = InstrParser::parseInstruction(&I);
        if (!pv.isUsed())
          continue;

        // check tracked types
        auto* st = pv.getObjStructType();
        if (!st || !globals.dbgInfo.isUsedStructType(st))
          continue;

        Variable* data = nullptr;
        if (pv.isField()) {
          //field
          auto [st2, idx] = pv.getStructInfo();
          if (st != st2){
            errs() << *st << " " << *st2 << "\n";
            report_fatal_error("not the same type - data");
          }
            
          auto* dataSf = globals.dbgInfo.getStructField(st, idx);
          if (globals.locals.inVariables(dataSf)) {
            // field
            data = globals.locals.getVariable(dataSf);
          } else {
            // objptr
            data = globals.locals.getVariable(st);
          }
        } else if (pv.isObjPtr()) {
          // obj
          data = globals.locals.getVariable(st);
        } else {
          report_fatal_error("not possible - either data or ptr");
        }

        globals.locals.addInstrInfo(&I, instrType, data, pv);
      }
    }
  }

  void addDatas() {
    for (auto* f : globals.functions.getAnalyzedFunctions()) {
      globals.setActiveFunction(f);
      addInstrInfo(f);
    }
  }

  Globals& globals;

public:
  DataParser(Globals& globals_) : globals(globals_) { addDatas(); }
};

} // namespace llvm
