#pragma once
#include "Common.h"
#include "analysis_util/Traversal.h"

namespace llvm {

template <typename Globals> class FunctionParser {
  static constexpr const char* GLOBAL_ANNOT = "llvm.global.annotations";

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

          globals.functions.insertAnnotatedFunction(annotatedFunction,
                                                    annotation);
        }
      }
    }
  }

  void insertNamedFunctions() {
    for (auto& F : M) {
      globals.functions.insertNamedFunction(&F);
    }
  }

  void insertAllAnalyzedFunctions() {
    for (auto* function : globals.functions.getAnalyzedFunctions()) {
      for (auto* f : globals.functions.getUnitFunctions(function)) {
        globals.functions.insertToAllAnalyzed(f);
      }
    }
  }

  virtual bool isKnownSkipFunction(Function* f) const {
    (void)(f);
    return false;
  }

  void insertSkipFunctions() {
    for (auto& F : M) {
      if (F.isIntrinsic() || F.isDeclaration())
        continue;

      // insert all skip functions
      auto* lastInstr = Traversal::getFunctionExitKey(&F);
      if (!isa<ReturnInst>(lastInstr) || isKnownSkipFunction(&F))
        globals.functions.insertSkipFunction(&F);
    }
  }

  Module& M;
  Globals& globals;

public:
  FunctionParser(Module& M_, Globals& globals_) : M(M_), globals(globals_) {}

  void parse(){
    insertSkipFunctions();
    insertAnnotatedFunctions();
    insertNamedFunctions();
    insertAllAnalyzedFunctions();
  }
};

} // namespace llvm