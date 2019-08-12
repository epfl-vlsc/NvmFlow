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

  auto* getValid(Instruction* i) {
    auto [st, idx] = getAnnotatedField(i);
    assert(st);
    return units.dbgInfo.getStructElement(st, idx);
  }

  auto* getObj(Variable* valid) { return units.dbgInfo.getStructObj(valid); }

  void insertVar(Instruction* i, InstructionType instrType) {
    if (auto [hasAnnot, annotation] = getAnnotatedField(i, FIELD_ANNOT);
        hasAnnot) {

      // find valid
      auto* valid = getValid(i);

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

      auto* diVar = getDILocalVariable(i);

      if (InstructionInfo::isUsedInstr(instrType))
        units.variables.insertInstruction(i, instrType, valid, diVar);
    }
  }

  void insertRead(LoadInst* li) { insertVar(li, InstructionType::None); }

  void insertWrite(StoreInst* si) {
    insertVar(si, InstructionType::WriteInstr);
  }

  void insertFlush(CallInst* ci, InstructionType instrType) {
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
  ValidParser(Units& units_) : units(units_) {
    for (auto* function : units.functions.getAnalyzedFunctions()) {
      units.setActiveFunction(function);
      insertFields(function);
    }
  }
};

} // namespace llvm