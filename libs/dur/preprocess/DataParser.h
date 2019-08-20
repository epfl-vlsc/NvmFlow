#pragma once
#include "Common.h"

#include "analysis_util/MemoryUtil.h"
#include "ds/Units.h"
#include "parser_util/ParserUtil.h"

namespace llvm {

class DataParser {
  static constexpr const char* FIELD_ANNOT = "DurableField";

  struct DataInfo {
    bool isUsed;
    Type* type;
    StructElement* se;
    bool annotated;
    AliasGroup* ag;
    DILocalVariable* diVar;
  };

  using InstructionType = typename InstructionInfo::InstructionType;

  auto* getObj(StructElement* se) { return units.dbgInfo.getStructObj(se); }

  DataInfo getVarInfo(Instruction* i, Value* v, bool loaded = false) {
    //errs() << *i << "\n";

    if (auto [hasAnnot, annotation] = getAnnotatedField(v, FIELD_ANNOT);
        hasAnnot) {
      // annotated field
      auto [st, idx] = getAnnotatedField(v);
      assert(st);

      auto* se = units.dbgInfo.getStructElement(st, idx);
      assert(se);

      auto* ptrType = se->getType();
      auto* type = ptrType->getPointerElementType();

      bool ann = true;
      auto* ag = units.variables.getAliasGroup(i, loaded);
      auto* diVar = getDILocalVar(units, v);

      /*
      errs() << "annotfield: ";
      errs() << *type << " " << se->getName() << " " << ann << " " << ag << " "
             << diVar << "\n";
       */
      return {true, type, se, ann, ag, diVar};
    } else if (auto [st, idx] = getField(v); st) {
      // field
      auto* se = units.dbgInfo.getStructElement(st, idx);
      assert(se);

      auto* type = se->getType();
      bool ann = false;
      auto* ag = units.variables.getAliasGroup(i, loaded);
      auto* diVar = getDILocalVar(units, v);

      /*
      errs() << "field: ";
      errs() << *type << " " << se->getName() << " " << ann << " " << ag << " "
             << diVar << "\n";
       */
      return {true, type, se, ann, ag, diVar};
    } else if (auto* type = v->getType(); type->isPointerTy()) {
      // ptr obj
      v = v->stripPointerCasts();

      auto* objPtrType = v->getType();
      // must be ptr
      assert(objPtrType->isPointerTy());
      auto* objType = objPtrType->getPointerElementType();
      if (auto* st = dyn_cast<StructType>(objType)) {
        //obj type
        errs() << *st << "\n";
        auto* se = units.dbgInfo.getStructElement(st);
        assert(se);

        auto* type = se->getType();
        bool ann = false;
        auto* ag = units.variables.getAliasGroup(i, loaded);
        auto* diVar = getDILocalVar(units, v);

        /*
        errs() << "obj: ";
        errs() << *type << " " << se << " " << ann << " " << ag
               << " " << diVar << "\n";
         */
        return {true, type, se, ann, ag, diVar};
      } else {
        //normal type
        auto* se = (StructElement*)nullptr;

        auto* type = objType;
        bool ann = false;
        auto* ag = units.variables.getAliasGroup(i, loaded);
        auto* diVar = getDILocalVar(units, v);
        /*
        errs() << "type: ";
        errs() << *type << " " << se << " " << ann << " " << ag
               << " " << diVar << "\n";
         */
        return {true, type, se, ann, ag, diVar};
      }
    }
    return {false, nullptr, nullptr, false, nullptr, nullptr};
  }

  auto* getSingleVariable(DataInfo& di) {
    auto [isUsed, type, se, annotated, ag, diVar] = di;
    if (!isUsed)
      return (SingleVariable*)nullptr;

    auto* sv =
        units.variables.insertSingleVariable(type, se, annotated, ag, diVar);

    return sv;
  }

  bool usesPointer(StoreInst* si) const {
    auto* val = si->getValueOperand();
    auto* valType = val->getType();
    return valType->isPointerTy();
  }

  void insertWrite(StoreInst* si) {
    if (!usesPointer(si)) {
      return;
    }

    auto instrType = InstructionType::WriteInstr;

    auto* ptrOpnd = si->getPointerOperand();
    auto ptrInfo = getVarInfo(si, ptrOpnd);

    auto* valOpnd = si->getValueOperand();
    auto valInfo = getVarInfo(si, valOpnd, true);

    auto* var = getSingleVariable(ptrInfo);
    if (!var)
      return;

    auto* loadVar = getSingleVariable(valInfo);

    units.variables.insertInstruction(si, instrType, var, loadVar);
  }

  void insertFlush(CallInst* ci, InstructionType instrType) {
    auto* arg0Opnd = ci->getArgOperand(0);
    auto ptrInfo = getVarInfo(ci, arg0Opnd);

    auto* var = getSingleVariable(ptrInfo);
    assert(var);

    units.variables.insertInstruction(ci, instrType, var);
  }

  void insertCall(CallInst* ci) {
    auto* callee = ci->getCalledFunction();

    if (!callee || callee->isIntrinsic() || callee->getName().equals("_Znwm") ||
        units.functions.isSkippedFunction(callee)) {
      return;
    } else if (units.functions.isPfenceFunction(callee)) {
      units.variables.insertInstruction(ci, InstructionInfo::PfenceInstr);
    } else if (units.functions.isVfenceFunction(callee)) {
      units.variables.insertInstruction(ci, InstructionInfo::VfenceInstr);
    } else if (units.functions.isFlushFunction(callee)) {
      insertFlush(ci, InstructionInfo::FlushInstr);
    } else if (units.functions.isFlushFenceFunction(callee)) {
      insertFlush(ci, InstructionInfo::FlushFenceInstr);
    } else {
      units.variables.insertInstruction(ci, InstructionInfo::IpInstr);
    }
  }

  void insertI(Instruction* i) {
    if (auto* si = dyn_cast<StoreInst>(i)) {
      insertWrite(si);
    } else if (auto* ci = dyn_cast<CallInst>(i)) {
      insertCall(ci);
    }
  }

  void insertPointers(Function* function) {
    for (auto* f : units.functions.getUnitFunctions(function)) {
      for (auto& I : instructions(*f)) {
        insertI(&I);
      }
    }
  }

  Units& units;

public:
  DataParser(Units& units_) : units(units_) {
    for (auto* function : units.functions.getAnalyzedFunctions()) {
      units.setActiveFunction(function);
      insertPointers(function);
    }
  }
};

} // namespace llvm