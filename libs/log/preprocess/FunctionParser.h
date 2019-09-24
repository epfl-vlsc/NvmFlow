#pragma once
#include "Common.h"

#include "analysis_util/MemoryUtil.h"
#include "ds/Units.h"

namespace llvm {

class FunctionParser {
  static constexpr const char* GLOBAL_ANNOT = "llvm.global.annotations";

  void addAnnotFuncs() {
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

          units.functions.addAnnotFunc(annotatedFunction,
                                                  annotation);
        }
      }
    }
  }

  void addNamedFuncs() {
    for (auto& F : M) {
      auto* f = &F;
      auto mangledName = f->getName();

      if (!units.dbgInfo.functionExists(mangledName)) {
        continue;
      }

      auto realName = units.dbgInfo.getFunctionName(mangledName);
      units.functions.addNamedFunc(f, realName);
    }

    // ensure tx are properly used
    units.functions.setTxMode();
  }

  Module& M;
  Units& units;

public:
  FunctionParser(Module& M_, Units& units_) : M(M_), units(units_) {
    addAnnotFuncs();
    addNamedFuncs();
  }
};

} // namespace llvm