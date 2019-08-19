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
    DILocalVariable* diVar;
  };

  using InstructionType = typename InstructionInfo::InstructionType;

  auto* getPtr(Value* v) {
    auto [st, idx] = getAnnotatedField(v);
    if (st)
      return units.dbgInfo.getStructElement(st, idx);
    else
      return (StructElement*)nullptr;
  }

  auto* getObj(StructElement* se) { return units.dbgInfo.getStructObj(se); }

  auto getVarInfo(Instruction* i, Value* v, InstructionType instrType) {
    /*
    if (auto [hasAnnot, annotation] = getAnnotatedField(i, FIELD_ANNOT);
        hasAnnot) {
      // annotated field

      // find valid
      auto* ptr = getPtr(v);

      auto*

      return DataInfo{se->getType(), se, true, };
    } else if ()
     */
    return 2;
  }

  void insertWrite(StoreInst* si) {
    auto instrType = InstructionType::WriteInstr;
    if (auto* ptrOpnd = si->getPointerOperand()) {
      auto ptrVar = getVarInfo(si, ptrOpnd, instrType);
    }

    if (auto* valOpnd = si->getValueOperand()) {
      auto valVar = getVarInfo(si, valOpnd, instrType);
    }
  }

  void insertFlush(CallInst* ci, InstructionType instrType) {
    if (auto* arg0Opnd = ci->getArgOperand(0)) {
      auto valVar = getVarInfo(ci, arg0Opnd, instrType);
    }
  }

  void insertCall(CallInst* ci) {
    auto* callee = ci->getCalledFunction();

    if (!callee || callee->isIntrinsic() ||
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