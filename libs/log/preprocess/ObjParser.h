#pragma once
#include "Common.h"

#include "analysis_util/MemoryUtil.h"
#include "ds/Units.h"

namespace llvm {

class ObjParser {
  using InstructionType = typename InstructionInfo::InstructionType;

  auto* getVar(Instruction* i, InstructionType instrType) {
    if (InstructionInfo::isLoggingBasedInstr(instrType)) {
      // obj
      if (auto* st = getObj(i)) {
        auto* obj = units.dbgInfo.getStructElement(st);
        assert(obj);
        if (units.variables.inVars(obj)) {
          return obj;
        }
      }
    }

    return (Variable*)nullptr;
  }

  void insertVar(Instruction* i, InstructionType instrType) {
    if (auto* var = getVar(i, instrType)) {
      auto* diVar = getDILocalVariable(i);
      units.variables.insertInstruction(i, instrType, var, diVar);
    }
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
    } else if (units.functions.isLoggingFunction(callee)) {
      insertLogging(ci, InstructionInfo::LoggingInstr);
    }
  }

  void insertI(Instruction* i) {
    if (auto* ci = dyn_cast<CallInst>(i)) {
      insertCall(ci);
    }
  }

  void insertFields(Function* function) {
    for (auto* f : units.functions.getUnitFunctions(function)) {
      for (auto& I : instructions(*f)) {
        auto* i = &I;
        if (!units.variables.isUsedInstruction(i))
          insertI(i);
      }
    }
  }

  Units& units;

public:
  ObjParser(Units& units_) : units(units_) {
    for (auto* function : units.functions.getAnalyzedFunctions()) {
      units.setActiveFunction(function);
      insertFields(function);
    }
  }
};

} // namespace llvm
