#pragma once
#include "Common.h"

#include "ds/InstrInfo.h"
#include "parser_util/InstrParser.h"

namespace llvm {

template <typename Globals, typename FuncSet, typename AliasInfo>
class DataParser {
  void addInstrInfo() {
    for (auto* f : fs) {
      for (auto& I : instructions(*f)) {
        // get instruction type
        auto instrType = InstrInfo::getInstrType(&I, globals);

        if (!InstrInfo::isUsedInstr(instrType))
          continue;

        // check if instruction registered before
        if (globals.locals.getInstrInfo(&I))
          continue;

        // parse variable based
        auto pv = InstrParser::parseVarLhs(&I);
        if (!pv.isUsed())
          continue;

        // check tracked types
        auto* st = pv.getObjStructType();
        if (!st)
          continue;

        // write to objptr does not change anything
        if (pv.isStoreInst() && pv.isObj())
          continue;

        Variable* data = nullptr;
        auto* alias = pv.getObjAlias();
        int setNo = ai.getSetNo(alias);

        if (!globals.dbgInfo.isUsedStructType(st) && pv.isCallInst()) {
          // flush field that is objptr
          auto* fieldType = pv.getFieldElementType();
          auto* stFieldType = dyn_cast<StructType>(fieldType);
          if (!stFieldType)
            continue;

          if (!globals.dbgInfo.isUsedStructType(stFieldType))
            continue;

          if (!globals.locals.inVariables(stFieldType, setNo))
            continue;

          data = globals.locals.getVariable(stFieldType, setNo);
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
          if (globals.locals.inVariables(dataSf, setNo)) {
            // field
            data = globals.locals.getVariable(dataSf, setNo);
          } else if (globals.locals.inVariables(st, setNo)) {
            // objptr
            data = globals.locals.getVariable(st, setNo);
          } else {
            continue;
          }
        } else if (pv.isObj() && globals.locals.inVariables(st, setNo)) {
          // obj
          data = globals.locals.getVariable(st, setNo);
        } else {
          //not matched to any df variable
          continue;
        }

        globals.locals.addInstrInfo(&I, instrType, data, pv);
      }
    }
  }

  Globals& globals;
  FuncSet& fs;
  AliasInfo& ai;

public:
  DataParser(Globals& globals_, FuncSet& fs_, AliasInfo& ai_)
      : globals(globals_), fs(fs_), ai(ai_) {
    addInstrInfo();
  }
};

} // namespace llvm
