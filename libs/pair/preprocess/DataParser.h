#pragma once
#include "Common.h"

#include "parser_util/InstrParser.h"
#include "ds/Globals.h"
#include "ds/InstrInfo.h"

namespace llvm {

class DataParser {
  using InstrType = typename InstrInfo::InstrType;

  void addVar(Instruction* i, InstrType instrType) {
    auto pv = InstrParser::parseInstruction(i, globals.dbgInfo);
    if (!pv.isUsed())
      return;

    auto* data = (Variable*)nullptr;

    if (pv.isObjPtr()) {
      // objptr
      auto* type = pv.getObjType();
      auto* st = dyn_cast<StructType>(type);
      data = globals.locals.getVariable(st);
    } else if (pv.isField()) {
      // data
      auto [st, idx] = pv.getStructInfo();

      auto* dataSf = globals.dbgInfo.getStructField(st, idx);

      if (globals.locals.inVariables(dataSf)) {
        // field
        data = globals.locals.getVariable(dataSf);
      } else {
        // objptr
        data = globals.locals.getVariable(st);
      }
    }

    globals.locals.addInstrInfo(i, instrType, data, pv);
  }

  void addWrite(StoreInst* si) { addVar(si, InstrInfo::WriteInstr); }

  void addFlush(CallInst* ci, InstrType instrType) { addVar(ci, instrType); }

  void addCall(CallInst* ci) {
    auto* callee = ci->getCalledFunction();

    if (globals.functions.isSkippedFunction(callee)) {
      return;
    } else if (globals.functions.isFlushFunction(callee)) {
      addFlush(ci, InstrInfo::FlushInstr);
    } else if (globals.functions.isFlushFenceFunction(callee)) {
      addFlush(ci, InstrInfo::FlushFenceInstr);
    }
  }

  void addI(Instruction* i) {
    if (auto* si = dyn_cast<StoreInst>(i)) {
      addWrite(si);
    } else if (auto* ci = dyn_cast<CallInst>(i)) {
      addCall(ci);
    }
  }

  void addDatas(Function* func) {
    for (auto* f : globals.functions.getUnitFunctions(func)) {
      for (auto& I : instructions(*f)) {
        if (!globals.locals.isUsedInstruction(&I))
          addI(&I);
      }
    }
  }

  Globals& globals;

public:
  DataParser(Globals& globals_) : globals(globals_) {
    for (auto* f : globals.functions.getAnalyzedFunctions()) {
      globals.setActiveFunction(f);
      addDatas(f);
    }
  }
};

} // namespace llvm
