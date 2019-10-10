#pragma once
#include "Common.h"

#include "ds/InstrInfo.h"
#include "parser_util/InstrParser.h"

namespace llvm {

template <typename Globals> class ValidParser {
  void addInstrInfo(Function* func) {
    std::set<StructType*> seenSts;
    for (auto* f : globals.functions.getUnitFunctions(func)) {
      for (auto& I : instructions(*f)) {
        // get instruction type
        auto instrType = InstrInfo::getInstrType(&I, globals);

        if (!InstrInfo::isUsedInstr(instrType))
          continue;

        // check non variable based parsing
        if (InstrInfo::isNonVarInstr(instrType)) {
          globals.locals.addInstrInfo(&I, instrType, nullptr, ParsedVariable());
          continue;
        }

        // parse variable based
        auto pv = InstrParser::parseInstruction(&I);
        if (!pv.isUsed() || !pv.isAnnotated() || !pv.isField())
          continue;

        // check annotation
        auto annot = pv.getAnnotation();
        if (!AnnotParser::isValidAnnotation(annot))
          continue;

        // check tracked types
        auto* st = pv.getObjStructType();
        if (!st || !globals.dbgInfo.isUsedStructType(st))
          continue;

        Variable* objVar = nullptr;
        // obj
        if (!seenSts.count(st)) {
          objVar = globals.locals.addVariable(st);
          seenSts.insert(st);
        } else {
          objVar = globals.locals.getVariable(st);
        }

        // valid
        auto [st2, idx] = pv.getStructInfo();
        if (st != st2) {
          errs() << *st << " " << *st2 << "\n";
          report_fatal_error("not the same type - valid");
        }
        auto* sf = globals.dbgInfo.getStructField(st, idx);
        Variable* valid = globals.locals.addVariable(sf);
        globals.locals.addSentinel(valid);

        // data
        Variable* data = nullptr;
        auto [parsedAnnot, useDcl] = AnnotParser::parseAnnotation(annot);
        if (!parsedAnnot.empty()) {
          // data
          auto* dataSf = globals.dbgInfo.getStructField(parsedAnnot);
          data = globals.locals.addVariable(dataSf);
        } else {
          // use obj
          data = objVar;
        }

        // pair
        assert(valid && data);
        globals.locals.addPair(data, valid, useDcl);

        // ii
        globals.locals.addInstrInfo(&I, instrType, valid, pv);
      }
    }
  }

  void addValids() {
    for (auto* f : globals.functions.getAnalyzedFunctions()) {
      globals.setActiveFunction(f);
      addInstrInfo(f);
    }
  }

  Globals& globals;

public:
  ValidParser(Globals& globals_) : globals(globals_) { addValids(); }
};

} // namespace llvm