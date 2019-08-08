#pragma once
#include "Common.h"

#include "analysis_util/MemoryUtil.h"
#include "ds/Units.h"

namespace llvm {

class WriteParser {
  using InstructionType = typename InstructionInfo::InstructionType;

  void insertWrite(StoreInst* si) {
    auto accessInfo = AccessType::getAccessInfo(si);
    if (accessInfo.isNone())
      return;
    errs() << accessInfo.getName() << "\n";

    InstructionType instrType = InstructionInfo::WriteInstr;
    if (accessInfo.isAnnotatedField()) {
      auto loadedInfo = AccessType::getLoadedVar(si);
      errs() << loadedInfo.getName() << "\n";
      // xxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    } else if (accessInfo.isField()) {
      auto [type, idx, varInfo] = accessInfo;
      // xxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    } else if (accessInfo.isObj()) {
      auto [type, idx, varInfo] = accessInfo;
      auto* ast = units.variables.getAliasSetTracker();
      assert(ast);
      auto* aliasSet = AccessType::getAliasSet(ast, si);
      assert(aliasSet);
      auto* se = units.dbgInfo.getStructElementFromType(type, idx);
      auto* svar =
          units.variables.insertSingleVariable(type, se, false, aliasSet);

      units.variables.insertInstruction(instrType, si, svar, nullptr);
    }

    /*
    auto [type, idx, varInfo] = AccessType::getAccessInfo(arg0Instr);

    auto* ast = units.variables.getAliasSetTracker();
    auto* aliasSet = AccessType::getAliasSet(ast, arg0Instr);

    if (!aliasSet) {
      // annotated field, ignore
      return;
    }

    // obj, field
    // insert lattice variable
    units.variables.insertVariable(aliasSet);

    // insert variable
    auto* se = units.dbgInfo.getStructElementFromType(type, idx);
    auto* svar =
        units.variables.insertSingleVariable(type, se, false, aliasSet);

    units.variables.insertInstruction(instrType, ci, svar, nullptr);
     */
  }

  void insertI(Instruction* i) {
    if (auto* si = dyn_cast<StoreInst>(i)) {
      insertWrite(si);
    }
  }

  void insertWriteVariables(Function* function) {
    for (auto* f : units.functions.getUnitFunctions(function)) {
      for (auto& I : instructions(*f)) {
        insertI(&I);
      }
    }
  }

  Units& units;

public:
  WriteParser(Units& units_) : units(units_) {
    for (auto* function : units.functions.getAnalyzedFunctions()) {
      units.setActiveFunction(function);
      insertWriteVariables(function);
    }
  }
};

} // namespace llvm
