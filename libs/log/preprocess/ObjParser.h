#pragma once
#include "Common.h"

#include "analysis_util/MemoryUtil.h"
#include "ds/Units.h"

namespace llvm {

class ObjParser {
  Units& units;

  auto* getDataVar(Instruction* i, InstructionInfo::InstructionType instrType) {
    if (InstructionInfo::isLoggingBasedInstr(instrType)) {
      // obj
      auto* uncastedArg0 = getUncasted(i);
      auto* argType = uncastedArg0->getType();
      // must be ptr
      assert(argType->isPointerTy());
      auto* objType = argType->getPointerElementType();
      if (auto* st = dyn_cast<StructType>(objType)) {
        auto* obj = units.dbgInfo.getStructElement(st);
        assert(obj);
        if (units.variables.inVars(obj)) {
          return obj;
        }
      }
    }

    return (Variable*)nullptr;
  }

  void insertVar(Instruction* i, InstructionInfo::InstructionType instrType) {
    auto* var = getDataVar(i, instrType);

    if (!var)
      return;

    // insert to ds
    units.variables.insertInstruction(instrType, i, var);
  }

  void insertLogging(CallInst* ci, InstructionInfo::InstructionType instrType) {
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

  void insertFields(Function* function, std::set<Function*>& visited) {
    visited.insert(function);

    for (auto& I : instructions(*function)) {
      auto* i = &I;

      if (!units.variables.isUsedInstruction(i))
        insertI(i);

      if (auto* ci = dyn_cast<CallInst>(i)) {
        auto* callee = ci->getCalledFunction();

        bool doIp = !callee->isDeclaration() && !visited.count(callee) &&
                    !units.functions.skipFunction(callee);
        if (doIp)
          insertFields(callee, visited);
      }
    }
  }

public:
  ObjParser(Units& units_) : units(units_) {
    for (auto* function : units.functions.getAnalyzedFunctions()) {
      units.setActiveFunction(function);
      std::set<Function*> visited;
      insertFields(function, visited);
    }
  }
};

} // namespace llvm
