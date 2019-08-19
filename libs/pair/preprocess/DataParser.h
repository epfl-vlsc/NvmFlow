#pragma once
#include "Common.h"

#include "analysis_util/MemoryUtil.h"
#include "ds/Units.h"
#include "parser_util/ParserUtil.h"

namespace llvm {

class DataParser {
  using InstructionType = typename InstructionInfo::InstructionType;

  auto* getVar(Instruction* i, InstructionType instrType) {
    if (auto [st, idx] = getField(i); st) {
      // field - data field is always unannotated

      // try field
      auto* data = units.dbgInfo.getStructElement(st, idx);

      if (units.variables.inDataSet(data)) {
        return data;
      }

      // try obj since field is not registered
      auto* obj = units.dbgInfo.getStructObj(data);
      if (units.variables.inVars(obj)) {
        return obj;
      }
    } else if (InstructionInfo::isFlushBasedInstr(instrType)) {
      // obj
      if (auto* st = getObj(i)) {
        errs() << "obj" << *i << "\n";
        auto* obj = units.dbgInfo.getStructElement(st);
        assert(obj);
        if (units.variables.inVars(obj)) {
          return obj;
        }
      }
    }

    return (Variable*)nullptr;
  }

  void insertVar(Instruction* i, Instruction* instr, InstructionType instrType) {
    if (auto* var = getVar(instr, instrType)) {
      errs() << var->getName() << "\n";
      auto* diVar = getDILocalVar(units, instr);

      units.variables.insertInstruction(i, instrType, var, diVar);
    }
  }

  void insertWrite(StoreInst* si) {
    insertVar(si, si, InstructionInfo::WriteInstr);
  }

  void insertFlush(CallInst* ci, InstructionType instrType) {
    auto* arg0 = ci->getArgOperand(0);
    if (auto* arg0Instr = dyn_cast<Instruction>(arg0)) {
      insertVar(ci, arg0Instr, instrType);
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
  DataParser(Units& units_) : units(units_) {
    for (auto* function : units.functions.getAnalyzedFunctions()) {
      units.setActiveFunction(function);
      insertFields(function);
    }
  }
};

} // namespace llvm
