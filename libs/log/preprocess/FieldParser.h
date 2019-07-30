#pragma once
#include "Common.h"

#include "analysis_util/MemoryUtil.h"
#include "ds/Units.h"

namespace llvm {

class FieldParser {
  static constexpr const char* FIELD_ANNOT = "LogField";

  using InstructionType = typename InstructionInfo::InstructionType;

  Units& units;

  auto* getVar(IntrinsicInst* ii) {
    auto [st, idx] = getFieldInfo(ii);
    assert(st);
    return units.dbgInfo.getStructElement(st, idx);
  }

  auto* getObj(Variable* var) { return units.dbgInfo.getStructObj(var); }

  auto* insertVar(Instruction* i) {
    auto* ii = getII(i);
    if (!ii) {
      return (Variable*)nullptr;
    }

    if (auto [annotation, hasAnnot] = isAnnotatedField(ii, FIELD_ANNOT);
        annotation.equals(FIELD_ANNOT) && hasAnnot) {

      // find var
      auto* var = getVar(ii);

      // insert to ds
      units.variables.insertVariable(var);

      // insert obj
      auto* obj = getObj(var);
      units.variables.insertObj(obj);

      return var;
    }

    return (Variable*)nullptr;
  }

  void insertInstructionIfVar(InstructionType instrType, Instruction* i,
                                Variable* var) {
    if (var)
      units.variables.insertInstruction(instrType, i, var);
  }

  void insertRead(LoadInst* li) { insertVar(li); }

  void insertWrite(StoreInst* si) {
    auto* var = insertVar(si);
    insertInstructionIfVar(InstructionInfo::WriteInstr, si, var);
  }

  void insertLogging(InstructionInfo::InstructionType instrType, CallInst* ci) {
    auto* arg0 = ci->getArgOperand(0);
    if (auto* arg0Instr = dyn_cast<Instruction>(arg0)) {
      auto* var = insertVar(arg0Instr);
      insertInstructionIfVar(instrType, ci, var);
    }
  }

  void insertCall(CallInst* ci) {
    auto* callee = ci->getCalledFunction();

    if (!callee || callee->isIntrinsic() ||
        units.functions.isSkippedFunction(callee)) {
      return;
    } else if (units.functions.isTxbeginFunction(callee)) {
      units.variables.insertInstruction(InstructionInfo::TxBegInstr, ci,
                                        nullptr);
    } else if (units.functions.isTxendFunction(callee)) {
      units.variables.insertInstruction(InstructionInfo::TxEndInstr, ci,
                                        nullptr);
    } else if (units.functions.isLoggingFunction(callee)) {
      insertLogging(InstructionInfo::LoggingInstr, ci);
    } else {
      units.variables.insertInstruction(InstructionInfo::IpInstr, ci, nullptr);
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

  void insertFields(Function* function, std::set<Function*>& visited) {
    visited.insert(function);

    for (auto& I : instructions(*function)) {
      auto* i = &I;
      insertI(i);

      if (auto* ci = dyn_cast<CallInst>(i)) {
        auto* callee = ci->getCalledFunction();
        bool doIp = !callee->isDeclaration() && !visited.count(callee) &&
                    !units.functions.skipFunction(callee);
        if (doIp) {
          insertFields(callee, visited);
        }
      }
    }
  }

public:
  FieldParser(Units& units_) : units(units_) {
    for (auto* function : units.functions.getAnalyzedFunctions()) {
      units.setActiveFunction(function);
      std::set<Function*> visited;
      insertFields(function, visited);
    }
  }
};

} // namespace llvm