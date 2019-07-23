#pragma once
#include "Common.h"

#include "analysis_util/MemoryUtil.h"
#include "ds/Units.h"

namespace llvm {

class ValidParser {
  static constexpr const char* FIELD_ANNOT = "pair";
  static constexpr const char* SCL_ANNOT = "scl";
  static constexpr const char* SEP = "-";

  Module& M;
  Units& units;

  std::pair<std::string, bool> parseAnnotation(StringRef annotation) {
    bool useDcl = annotation.contains(SCL_ANNOT) ? false : true;
    auto [_, name] = annotation.rsplit(SEP);
    return {name.str(), useDcl};
  }

  void insertII(Instruction* i, InstructionInfo::InstructionType instrType) {
    auto* ii = getII(i);
    if (!ii) {
      return;
    }

    if (auto [annotation, hasAnnot] = isAnnotatedField(ii, FIELD_ANNOT);
        hasAnnot) {
      // find valid
      auto [st, idx] = getFieldInfo(ii);
      assert(st);

      auto* valid = units.dbgInfo.getStructElement(st, idx);

      // find data
      auto [dataStrIdx, useDcl] = parseAnnotation(annotation);
      auto* data = units.dbgInfo.getStructElement(dataStrIdx);

      // insert to ds
      units.variables.insertPair(data, valid, useDcl);
      units.variables.insertInstruction(instrType, i, valid);
    }
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
    } else if (units.functions.isPfenceFunction(callee)) {
      units.variables.insertInstruction(InstructionInfo::PfenceInstr, ci,
                                        nullptr);
    } else if (units.functions.isVfenceFunction(callee)) {
      units.variables.insertInstruction(InstructionInfo::VfenceInstr, ci,
                                        nullptr);
    } else if (units.functions.isFlushFunction(callee)) {
      insertFlush(ci, InstructionInfo::FlushInstr);
    } else if (units.functions.isFlushFenceFunction(callee)) {
      insertFlush(ci, InstructionInfo::FlushFenceInstr);
    } else {
      units.variables.insertInstruction(InstructionInfo::IpInstr, ci, nullptr);
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
        if (!callee->isDeclaration() && !visited.count(callee) &&
            !units.functions.isSkippedFunction(callee)) {
          insertFields(callee, visited);
        }
      }
    }
  }

public:
  ValidParser(Module& M_, Units& units_) : M(M_), units(units_) {
    for (auto* function : units.functions.getAnalyzedFunctions()) {
      units.setActiveFunction(function);
      std::set<Function*> visited;
      insertFields(function, visited);
    }
  }
};

} // namespace llvm