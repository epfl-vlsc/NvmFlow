#pragma once
#include "Common.h"

#include "analysis_util/MemoryUtil.h"
#include "data_util/DbgInfo.h"

#include "ds/Units.h"

namespace llvm {

class Parser {
  static constexpr const char* GLOBAL_ANNOT = "llvm.global.annotations";
  static constexpr const char* FIELD_ANNOT = "LogField";

  Module& M;
  ModulePass* pass;
  DbgInfo& dbgInfo;
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

          units.insertAnnotatedFunction(annotatedFunction, annotation);
        }
      }
    }
  }

  void insertNamedFunctions() {
    for (auto& F : M) {
      auto* f = &F;
      if (f->isIntrinsic()) {
        continue;
      }
      auto mangledName = f->getName();
      // auto realName = dbgInfo.getFunctionName(mangledName);
      units.insertNamedFunction(f, mangledName);
    }
  }

  void insertSi(StoreInst* si) {
    auto* ii = getII(si);
    if (!ii || !isAnnotatedField(ii, FIELD_ANNOT)) {
      return;
    }

    auto [st, idx] = getFieldInfo(ii);

    units.insertVariable(si, st, idx);
  }

  void insertCi(CallInst* ci) {
    auto* callee = ci->getCalledFunction();
    if (!callee || !units.isLoggingFunction(callee)) {
      return;
    }

    auto* arg0 = ci->getArgOperand(0);
    auto* ii = getII(arg0);
    llvm::errs() << *ii << "\n";

    if (!ii || !isAnnotatedField(ii, FIELD_ANNOT)) {
      return;
    }

    auto [st, idx] = getFieldInfo(ii);
    units.insertVariable(ci, st, idx);
  }

  void insertI(Instruction* i) {
    if (auto* si = dyn_cast<StoreInst>(i)) {
      insertSi(si);
    } else if (auto* ci = dyn_cast<CallInst>(i)) {
      insertCi(ci);
    }
  }

  void insertAnnotatedVariables() {
    for (auto& F : M) {
      auto* f = &F;
      if (units.isSkippedFunction(f) || f->isDeclaration()) {
        continue;
      }

      for (auto& BB : F) {
        for (auto& I : BB) {
          insertI(&I);
        }
      }
    }
  }

public:
  Parser(Module& M_, ModulePass* pass_, DbgInfo& dbgInfo_, Units& units_)
      : M(M_), pass(pass_), dbgInfo(dbgInfo_), units(units_) {
    // parse functions
    insertAnnotatedFunctions();
    insertNamedFunctions();

    // parse variables
    insertAnnotatedVariables();
  }
}; // namespace llvm

} // namespace llvm