#pragma once
#include "Common.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Transforms/Scalar/ADCE.h"
#include "llvm/Transforms/Scalar/SCCP.h"
#include "llvm/Transforms/Scalar/SROA.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"

namespace llvm {

class UseNvmPass {
  Module& M;
  PassBuilder PB;
  FunctionAnalysisManager FA;
  FunctionPassManager FPM;

  void run() {
    for (auto& F : M) {
      if (F.isDeclaration() || F.isIntrinsic())
        continue;

      FPM.run(F, FA);
    }
  }

public:
  UseNvmPass(Module& M_) : M(M_), FPM(false) {
    PB.registerFunctionAnalyses(FA);
    FPM.addPass(SROA());
    FPM.addPass(SCCPPass());
    FPM.addPass(ADCEPass());
    FPM.addPass(SimplifyCFGPass());

    run();
  }
};

} // namespace llvm