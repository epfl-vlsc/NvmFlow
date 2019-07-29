#pragma once
#include "Common.h"

#include "analysis_util/MemoryUtil.h"
#include "ds/Units.h"

namespace llvm {

class DataParser {
  Units& units;

  auto* getDataVar(Instruction* i, InstructionInfo::InstructionType instrType) {
    if (auto* gepi = getGEPI(i)) {
      // field - data field is always unannotated
      auto [st, idx] = getFieldInfo(gepi);

      // try field
      auto* data = units.dbgInfo.getStructElement(st, idx);
      if (units.variables.inDataSet(data)) {
        return data;
      }

      // try obj
      auto* obj = units.dbgInfo.getStructObj(data);
      if (units.variables.inVars(obj)) {
        return obj;
      }
    } else if (InstructionInfo::isFlushBasedInstr(instrType)) {
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

  void insertII(Instruction* i, InstructionInfo::InstructionType instrType) {
    auto* data = getDataVar(i, instrType);

    if (!data)
      return;

    // insert to ds
    units.variables.insertInstruction(instrType, i, data);
  }

  void insertWrite(StoreInst* si) { insertII(si, InstructionInfo::WriteInstr); }

  void insertFlush(CallInst* ci, InstructionInfo::InstructionType instrType) {
    auto* arg0 = ci->getArgOperand(0);
    if (auto* arg0Instr = dyn_cast<Instruction>(arg0)) {
      insertII(arg0Instr, instrType);
    }
  }

  void insertCall(CallInst* ci) {
    auto* callee = ci->getCalledFunction();

    if (!callee || callee->isIntrinsic() ||
        units.functions.isSkippedFunction(callee)) {
      return;
    } else if (units.functions.isFlushFunction(callee)) {
      insertFlush(ci, InstructionInfo::FlushInstr);
    } else if (units.functions.isFlushFenceFunction(callee)) {
      insertFlush(ci, InstructionInfo::FlushFenceInstr);
    }
  }

  void insertI(Instruction* i) {
    if (auto* si = dyn_cast<StoreInst>(i)) {
      insertWrite(si);
    } else if (auto* ci = dyn_cast<CallInst>(i)) {
      insertCall(ci);
    }
  }

  void insertFields(Function* function, std::set<Function*>& visited) {
    visited.insert(function);

    for (auto& I : instructions(*function)) {
      auto* i = &I;
      insertI(i);

      if (auto* ci = dyn_cast<CallInst>(i)) {
        auto* callee = ci->getCalledFunction();

        bool analyzeInstruction = !callee->isDeclaration() &&
                                  !visited.count(callee) &&
                                  !units.functions.isSkippedFunction(callee) &&
                                  !units.variables.isUsedInstruction(i);

        if (analyzeInstruction)
          insertFields(callee, visited);
      }
    }
  }

public:
  DataParser(Units& units_) : units(units_) {
    for (auto* function : units.functions.getAnalyzedFunctions()) {
      units.setActiveFunction(function);
      std::set<Function*> visited;
      insertFields(function, visited);
    }
  }
};

} // namespace llvm
