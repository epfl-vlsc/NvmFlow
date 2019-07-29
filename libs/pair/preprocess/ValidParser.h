#pragma once
#include "Common.h"

#include "analysis_util/MemoryUtil.h"
#include "ds/Units.h"

namespace llvm {

class ValidParser {
  static constexpr const char* FIELD_ANNOT = "pair";
  static constexpr const char* SCL_ANNOT = "scl";
  static constexpr const char* SEP = "-";

  using InstructionType = typename InstructionInfo::InstructionType;

  Units& units;

  std::pair<std::string, bool> parseAnnotation(StringRef annotation) {
    bool useDcl = annotation.contains(SCL_ANNOT) ? false : true;
    auto [_, name] = annotation.rsplit(SEP);
    return {name.str(), useDcl};
  }

  std::pair<Variable*, bool> getData(StringRef annotation, StructType* st) {
    auto [dataStrIdx, useDcl] = parseAnnotation(annotation);
    Variable* data = nullptr;
    if (dataStrIdx.empty()) {
      // whole object
      data = units.dbgInfo.getStructElement(st);
    } else {
      // field
      data = units.dbgInfo.getStructElement(dataStrIdx);
    }

    assert(data);
    return {data, useDcl};
  }

  auto* getValid(IntrinsicInst* ii) {
    auto [st, idx] = getFieldInfo(ii);
    assert(st);
    return units.dbgInfo.getStructElement(st, idx);
  }

  auto* getObj(Variable* valid) { return units.dbgInfo.getStructObj(valid); }

  auto* insertVar(Instruction* i) {
    auto* ii = getII(i);
    if (!ii) {
      return (Variable*)nullptr;
    }

    if (auto [annotation, hasAnnot] = isAnnotatedField(ii, FIELD_ANNOT);
        hasAnnot) {

      // find valid
      auto* valid = getValid(ii);

      // find data
      auto [data, useDcl] = getData(annotation, valid->getStType());

      // insert to ds
      units.variables.insertPair(data, valid, useDcl);

      // insert obj
      auto* objv = getObj(valid);
      auto* objd = getObj(data);
      units.variables.insertObj(objv);
      if (objv != objd)
        units.variables.insertObj(objd);

      return valid;
    }

    return (Variable*)nullptr;
  }

  void insertInstructionIfValid(InstructionType instrType, Instruction* i,
                                Variable* valid) {
    if (valid)
      units.variables.insertInstruction(instrType, i, valid);
  }

  void insertRead(LoadInst* li) { insertVar(li); }

  void insertWrite(StoreInst* si) {
    auto* valid = insertVar(si);
    insertInstructionIfValid(InstructionInfo::WriteInstr, si, valid);
  }

  void insertFlush(InstructionInfo::InstructionType instrType, CallInst* ci) {
    auto* arg0 = ci->getArgOperand(0);
    if (auto* arg0Instr = dyn_cast<Instruction>(arg0)) {
      auto* valid = insertVar(arg0Instr);
      insertInstructionIfValid(instrType, ci, valid);
    }
  }

  void insertCall(CallInst* ci) {
    auto* callee = ci->getCalledFunction();

    if (!callee || callee->isIntrinsic() ||
        units.functions.isSkippedFunction(callee)) {
      return;
    } else if (units.functions.isPfenceFunction(callee)) {
      units.variables.insertInstruction(InstructionInfo::PfenceInstr, ci,
                                        nullptr);
    } else if (units.functions.isVfenceFunction(callee)) {
      units.variables.insertInstruction(InstructionInfo::VfenceInstr, ci,
                                        nullptr);
    } else if (units.functions.isFlushFunction(callee)) {
      insertFlush(InstructionInfo::FlushInstr, ci);
    } else if (units.functions.isFlushFenceFunction(callee)) {
      insertFlush(InstructionInfo::FlushFenceInstr, ci);
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
        if (!callee->isDeclaration() && !visited.count(callee) &&
            !units.functions.isSkippedFunction(callee)) {
          insertFields(callee, visited);
        }
      }
    }
  }

public:
  ValidParser(Units& units_) : units(units_) {
    for (auto* function : units.functions.getAnalyzedFunctions()) {
      units.setActiveFunction(function);
      std::set<Function*> visited;
      insertFields(function, visited);
    }
  }
};

} // namespace llvm