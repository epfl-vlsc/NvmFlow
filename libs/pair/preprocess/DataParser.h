#pragma once
#include "Common.h"

#include "ds/InstrInfo.h"
#include "parser_util/InstrParser.h"

namespace llvm {

template <typename Globals> class DataParser {
  void addInstrInfo(Function* func) {
    std::set<StructType*> seenSts;
    for (auto* f : globals.functions.getUnitFunctions(func)) {
      for (auto& I : instructions(*f)) {
        // get instruction type
        auto instrType = InstrInfo::getInstrType(&I, globals);

        if (!InstrInfo::isUsedInstr(instrType))
          continue;

        // check if instruction registered before
        if (globals.locals.getInstrInfo(&I))
          continue;

        // parse variable based
        auto pv = InstrParser::parseInstruction(&I);
        if (!pv.isUsed())
          continue;

        // check tracked types
        auto* st = pv.getObjStructType();
        if (!st)
          continue;

        // write to objptr does not change anything
        if (pv.isWriteInst() && pv.isObjPtr())
          continue;

        Variable* data = nullptr;

        if (!globals.dbgInfo.isUsedStructType(st) && pv.isCallInst()) {
          // flush field that is objptr
          auto* fieldType = pv.getFieldElementType();
          auto* stFieldType = dyn_cast<StructType>(fieldType);
          if (!stFieldType)
            continue;

          if (!globals.dbgInfo.isUsedStructType(stFieldType))
            continue;

          data = globals.locals.getVariable(stFieldType);
          globals.locals.addInstrInfo(&I, instrType, data, pv);
          continue;
        } else if (!globals.dbgInfo.isUsedStructType(st)) {
          continue;
        }

        if (pv.isField()) {
          // field
          auto [st2, idx] = pv.getStructInfo();
          if (st != st2) {
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
