#pragma once
#include "Common.h"
#include "analysis_util/Traversal.h"
#include "ds/Globals.h"

namespace llvm {

class FunctionParser {
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
      auto* f = &F;
      auto mangledName = f->getName();

      if (!globals.dbgInfo.functionExists(mangledName)) {
        continue;
      }

      auto realName = globals.dbgInfo.getFunctionName(mangledName);
      globals.functions.insertNamedFunction(f, realName);
    }
  }

  void insertAllAnalyzedFunctions() {
    for (auto* function : globals.functions.getAnalyzedFunctions()) {
      for (auto* f : globals.functions.getUnitFunctions(function)) {
        globals.functions.insertToAllAnalyzed(f);
      }
    }
  }

  void insertSkipFunctions() {
    for (auto& F : M) {
      if (F.isIntrinsic() || F.isDeclaration())
        continue;

      auto* lastInstr = Traversal::getFunctionExitKey(&F);
      if (!lastInstr)
        globals.functions.insertSkipFunction(&F);
    }
  }

  Module& M;
  Globals& globals;

public:
  FunctionParser(Module& M_, Globals& globals_) : M(M_), globals(globals_) {
    insertSkipFunctions();
    insertAnnotatedFunctions();
    insertNamedFunctions();
    insertAllAnalyzedFunctions();
  }
};

} // namespace llvm