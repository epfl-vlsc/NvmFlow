#pragma once
#include "Common.h"

#include "ds/LogUnits.h"

namespace llvm {

class LogParser {
  static const constexpr char* GLOBAL_ANNOT = "llvm.global.annotations";

  Module& M;
  LogUnits& units;

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
      units.insertNamedFunction(&F);
    }
  }

public:
  LogParser(Module& M_, LogUnits& units_) : M(M_), units(units_) {
    insertAnnotatedFunctions();
    insertNamedFunctions();
  }
};

} // namespace llvm