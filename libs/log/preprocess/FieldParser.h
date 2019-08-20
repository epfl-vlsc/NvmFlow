#pragma once
#include "Common.h"

#include "analysis_util/MemoryUtil.h"
#include "ds/Units.h"

namespace llvm {

class FieldParser {
  static constexpr const char* FIELD_ANNOT = "LogField";

  using InstructionType = typename InstructionInfo::InstructionType;

  auto* getVar(Instruction* i) {
    auto [st, idx] = getAnnotatedField(i);
    assert(st);
    return units.dbgInfo.getStructElement(st, idx);
  }

  auto* getObj(Variable* var) { return units.dbgInfo.getStructObj(var); }

  void insertVar(Instruction* i, InstructionType instrType) {
    if (auto [hasAnnot, annotation] = getAnnotatedField(i, FIELD_ANNOT);
        hasAnnot && annotation.equals(FIELD_ANNOT)) {

      // find var
      auto* var = getVar(i);

      // insert to ds
      units.variables.insertVariable(var);

      // insert obj
      auto* obj = getObj(var);
      units.variables.insertObj(obj);

      // auto* diVar = getDILocalVariable(i);
      DILocalVariable* diVar = nullptr;

      if (InstructionInfo::isUsedInstr(instrType))
        units.variables.insertInstruction(i, instrType, var, diVar);
    }
  }

  void insertRead(LoadInst* li) { insertVar(li, InstructionType::None); }

  void insertWrite(StoreInst* si) {
    insertVar(si, InstructionType::WriteInstr);
  }

  void insertLogging(CallInst* ci, InstructionType instrType) {
    auto* arg0 = ci->getArgOperand(0);
    if (auto* arg0Instr = dyn_cast<Instruction>(arg0)) {
      insertVar(arg0Instr, instrType);
    }
  }

  void insertCall(CallInst* ci) {
    auto* callee = ci->getCalledFunction();

    if (!callee || callee->isIntrinsic() ||
        units.functions.isSkippedFunction(callee)) {
      return;
    } else if (units.functions.isTxbeginFunction(callee)) {
      units.variables.insertInstruction(ci, InstructionInfo::TxBegInstr);
    } else if (units.functions.isTxendFunction(callee)) {
      units.variables.insertInstruction(ci, InstructionInfo::TxEndInstr);
    } else if (units.functions.isLoggingFunction(callee)) {
      insertLogging(ci, InstructionInfo::LoggingInstr);
    } else {
      units.variables.insertInstruction(ci, InstructionInfo::IpInstr);
    }
  }

  void insertI(Instruction* i) {
    if (auto* si = dyn_cast<StoreInst>(i)) {
      insertWrite(si);
    } else if (auto* li = dyn_cast<LoadInst>(i)) {
      insertRead(li);
    } else if (auto* ci = dyn_cast<CallInst>(i)) {
      insertCall(ci);
    }
  }

  void insertFields(Function* function) {
    for (auto* f : units.functions.getUnitFunctions(function)) {
      for (auto& I : instructions(*f)) {
        insertI(&I);
      }
    }
  }

  Units& units;

public:
  FieldParser(Units& units_) : units(units_) {
    for (auto* function : units.functions.getAnalyzedFunctions()) {
      units.setActiveFunction(function);
      insertFields(function);
    }
  }
};

} // namespace llvm