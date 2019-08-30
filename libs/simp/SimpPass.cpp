#include "SimpPass.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "llvm/Transforms/Scalar/ADCE.h"
#include "llvm/Transforms/Scalar/SCCP.h"
#include "llvm/Transforms/Scalar/SROA.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"

namespace llvm {

void SimpPass::print(raw_ostream& OS, const Module* m) const {
  OS << "print bugs here\n";
}

bool SimpPass::runOnModule(Module& M) {
  PassBuilder PB;
  FunctionPassManager FPM;
  FunctionAnalysisManager FA;
  PB.registerFunctionAnalyses(FA);

  FPM.addPass(SCCPPass());
  FPM.addPass(ADCEPass());
  FPM.addPass(SimplifyCFGPass());

  for (auto& F : M) {
    if (F.isDeclaration() || F.isIntrinsic())
      continue;
    FPM.run(F, FA);
  }

  return true;
}

void SimpPass::getAnalysisUsage(AnalysisUsage& AU) const {}

char SimpPass::ID = 0;
RegisterPass<SimpPass> X("simp", "Simp pass");

} // namespace llvm