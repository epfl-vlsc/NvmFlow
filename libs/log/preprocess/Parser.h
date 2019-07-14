#pragma once
#include "Common.h"

#include "analysis_util/MemoryUtil.h"

#include "ds/Units.h"

namespace llvm {

class Parser {
  static constexpr const char* GLOBAL_ANNOT = "llvm.global.annotations";
  static constexpr const char* FIELD_ANNOT = "LogField";

  Module& M;
  Units& units;

  void insertAnnotatedFunctions() {
    for (Module::global_iterator I = M.global_begin(), E = M.global_end();
         I != E; ++I) {
      if (I->getName() == GLOBAL_ANNOT) {
        ConstantArray* CA = dyn_cast<ConstantArray>(I->getOperand(0));
        for (auto OI = CA->op_begin(); OI != CA->op_end(); ++OI) {
          ConstantStruct* CS = dyn_cast<ConstantStruct>(OI->get());
          GlobalVariable* AnnotationGL =
              dyn_cast<GlobalVariable>(CS->getOperand(1)->getOperand(0));
          StringRef annotation =
              dyn_cast<ConstantDataArray>(AnnotationGL->getInitializer())
                  ->getAsCString();

          Function* annotatedFunction =
              dyn_cast<Function>(CS->getOperand(0)->getOperand(0));

          units.functions.insertAnnotatedFunction(annotatedFunction,
                                                  annotation);
        }
      }
    }
  }

  void insertNamedFunctions() {
    for (auto& F : M) {
      auto* f = &F;
      auto mangledName = f->getName();

      if (!units.dbgInfo.functionExists(mangledName)) {
        continue;
      }

      auto realName = units.dbgInfo.getFunctionName(mangledName);
      units.functions.insertNamedFunction(f, realName);
    }

    // ensure tx are properly used
    units.functions.setTxMode();
  }

  void insertSi(StoreInst* si) {
    auto* ii = getII(si);
    if (!ii || !isAnnotatedField(ii, FIELD_ANNOT)) {
      return;
    }

    auto [st, idx] = getFieldInfo(ii);

    auto* variable = units.activeFunction->insertVariable(st, idx);
    assert(variable);

    units.activeFunction->insertInstruction(InstructionInfo::WriteInstr, si,
                                            variable);
  }

  void insertLogging(CallInst* ci) {
    auto* arg0 = ci->getArgOperand(0);
    auto* ii = getII(arg0);

    // todo get struct
    if (!ii || !isAnnotatedField(ii, FIELD_ANNOT)) {
      return;
    }

    // get field
    auto [st, idx] = getFieldInfo(ii);

    // get variable
    auto* variable = units.activeFunction->insertVariable(st, idx);
    assert(variable);

    units.activeFunction->insertInstruction(InstructionInfo::LoggingInstr, ci,
                                            variable);

    units.activeFunction->insertVariable(st, -1);
  }

  void insertCi(CallInst* ci) {
    auto* callee = ci->getCalledFunction();

    if (!callee || callee->isIntrinsic() ||
        units.functions.isSkippedFunction(callee)) {
      return;
    } else if (units.functions.isTxbeginFunction(callee)) {
      units.activeFunction->insertInstruction(InstructionInfo::TxBegInstr, ci,
                                              nullptr);
    } else if (units.functions.isTxendFunction(callee)) {
      units.activeFunction->insertInstruction(InstructionInfo::TxEndInstr, ci,
                                              nullptr);
    } else if (units.functions.isLoggingFunction(callee)) {
      // parse log
      insertLogging(ci);
    } else {
      // go to callee
      units.activeFunction->insertInstruction(InstructionInfo::CallInstr, ci,
                                              nullptr);
    }
  }

  void insertI(Instruction* i) {
    if (auto* si = dyn_cast<StoreInst>(i)) {
      insertSi(si);
    } else if (auto* ci = dyn_cast<CallInst>(i)) {
      insertCi(ci);
    }
  }

  void insertFields() {
    for (auto* function : units.functions.getAnalyzedFunctions()) {
      units.setActiveFunction(function);
      for (auto& BB : *function) {
        for (auto& I : BB) {
          insertI(&I);
        }
      }
    }
  }

  void insertCiObj(CallInst* ci) {
    auto* callee = ci->getCalledFunction();

    if (!callee || callee->isIntrinsic() ||
        units.functions.isSkippedFunction(callee)) {
      return;
    } else if (units.functions.isLoggingFunction(callee)) {
      // parse log
      auto* arg0 = ci->getArgOperand(0);
      auto* uncastedArg0 = getUncasted(arg0);
      auto* argType = uncastedArg0->getType();
      // must be ptr
      assert(argType->isPointerTy());
      auto* objType = argType->getPointerElementType();
      if (auto* st = dyn_cast<StructType>(objType)) {
        auto* variable = units.activeFunction->insertVariable(st, -1);
        assert(variable);

        units.activeFunction->insertInstruction(InstructionInfo::LoggingInstr,
                                                ci, variable);
      }
    }
  }

  void insertObjects() {
    for (auto* function : units.functions.getAnalyzedFunctions()) {
      units.setActiveFunction(function);
      for (auto& BB : *function) {
        for (auto& I : BB) {
          auto* i = &I;
          if (units.activeFunction->isUsedInstruction(i)) {
            continue;
          }

          if (auto* ci = dyn_cast<CallInst>(i)) {
            insertCiObj(ci);
          }
        }
      }
    }
  }

public:
  Parser(Module& M_, Units& units_) : M(M_), units(units_) {
    // parse functions
    insertAnnotatedFunctions();
    insertNamedFunctions();

    // parse variables
    insertFields();
    insertObjects();
  }
};

} // namespace llvm