#pragma once
#include "Common.h"

#include "ds/InstrInfo.h"
#include "parser_util/InstrParser.h"

namespace llvm {

template <typename Globals> class ValidParser {
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
    } else if (auto* ii = dyn_cast<InvokeInst>(i)) {
      return InstrType::IpInstr;
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