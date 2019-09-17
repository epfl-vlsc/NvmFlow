#pragma once
#include "Common.h"

#include "ds/InstrInfo.h"
#include "parser_util/InstrParser.h"
#include "llvm/ADT/EquivalenceClasses.h"

namespace llvm {

template <typename Globals> class AliasParser {
  using InstrType = typename InstrInfo::InstrType;

  static constexpr const char* DurableField = "DurableField";
  /*
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

    void addRead(LoadInst* li) { addVar(li, InstrType::None); }

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
      } else if (auto* li = dyn_cast<LoadInst>(i)) {
        // addRead(li);
      } else if (auto* ci = dyn_cast<CallInst>(i)) {
        addCall(ci);
      }
    }

    void addValids(Function* func) {
      for (auto* f : globals.functions.getUnitFunctions(func)) {
        for (auto& I : instructions(*f)) {
          addI(&I);
        }
      }
    }
  
  auto createAliasSets(Function* func, std::set<Type*>& ptrTypes,
                       std::set<StructType*>& structTypes) {
                        
  }
  */

  auto getUsedTypes(FunctionSet& funcSet) {
    std::set<Type*> ptrTypes;
    std::set<StructType*> structTypes;

    for (auto* f : funcSet) {
      for (auto& I : instructions(*f)) {
        // parse instr
        auto pv = InstrParser::parseInstruction(&I);
        if (!pv.isUsed() || !pv.isAnnotated())
          continue;

        // check annotation type
        auto annotation = pv.getAnnotation();
        if (!annotation.equals(DurableField))
          continue;

        // get ptr type
        auto* ptrType = pv.getType();
        assert(ptrType->isPointerTy());
        ptrTypes.insert(ptrType);

        // get struct type
        auto* structType = pv.getStructType();
        structTypes.insert(structType);
      }
    }

    return std::pair(ptrTypes, structTypes);
  }

  Globals& globals;

public:
  AliasParser(Globals& globals_) : globals(globals_) {
    for (auto* f : globals.functions.getAnalyzedFunctions()) {
      globals.setActiveFunction(f);
      auto funcSet = globals.functions.getUnitFunctionSet(f);

      auto [ptrTypes, structTypes] = getUsedTypes(funcSet);

      //add to global dbg info
      globals.dbgInfo.addDbgInfoFunctions(funcSet, structTypes);

      /*
      auto aliasSets = createAliasSets(f, ptrTypes, structTypes);
      createVariables(f, aliasSets);
      */
    }
  }
};

} // namespace llvm