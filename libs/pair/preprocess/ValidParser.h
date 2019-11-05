#pragma once
#include "Common.h"

#include "AnnotParser.h"
#include "ds/InstrInfo.h"
#include "parser_util/InstrParser.h"

namespace llvm {

template <typename Globals, typename FuncSet, typename AliasInfo>
class ValidParser {
  void addInstrInfo() {
    std::set<std::pair<StructType*, int>> seenSts;
    for (auto* f : fs) {
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
        auto pv = InstrParser::parseVarLhs(&I);
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
        auto* alias = pv.getObjAlias();
        int setNo = ai.getSetNo(alias);
        // obj
        if (!seenSts.count({st, setNo})) {
          objVar = globals.locals.addVariable(st, setNo);
          seenSts.insert({st, setNo});
        } else {
          objVar = globals.locals.getVariable(st, setNo);
        }

        // valid
        auto [st2, idx] = pv.getStructInfo();
        if (st != st2) {
          errs() << *st << " " << *st2 << "\n";
          report_fatal_error("not the same type - valid");
        }
        auto* sf = globals.dbgInfo.getStructField(st, idx);
        Variable* valid = globals.locals.addVariable(sf, setNo);
        globals.locals.addSentinel(valid);

        // data
        Variable* data = nullptr;
        auto [parsedAnnot, useDcl] = AnnotParser::parseAnnotation(annot);
        if (!parsedAnnot.empty()) {
          // data
          auto* dataSf = globals.dbgInfo.getStructField(parsedAnnot);
          data = globals.locals.addVariable(dataSf, setNo);
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

  Globals& globals;
  FuncSet& fs;
  AliasInfo& ai;

public:
  ValidParser(Globals& globals_, FuncSet& fs_, AliasInfo& ai_)
      : globals(globals_), fs(fs_), ai(ai_) {
    addInstrInfo();
  }
};

} // namespace llvm