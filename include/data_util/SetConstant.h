#pragma once
#include "Common.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Transforms/Scalar/ADCE.h"
#include "llvm/Transforms/Scalar/SCCP.h"
#include "llvm/Transforms/Scalar/SROA.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"

namespace llvm {

class SetConstant {
  static constexpr const char* Str[] = {"useNvm", "use_nvm"};

  bool isUseNvm(StringRef var) {
    for (auto* str : Str) {
      if (var.equals(str))
        return true;
    }
    return false;
  }

  auto* getTrueVal() {
    auto& context = M.getContext();
    auto* boolType = Type::getInt8Ty(context);
    auto* trueVal = ConstantInt::get(boolType, 1);
    return trueVal;
  }

  void modifyStores(AllocaInst* ai) {
    for (auto* user : ai->users()) {
      if (auto* si = dyn_cast<StoreInst>(user)) {
        errs() << *si << "\n";

        auto* trueVal = getTrueVal();
        auto* ptrOpnd = si->getPointerOperand();
        auto& DL = M.getDataLayout();
        auto alignment = ptrOpnd->getPointerAlignment(DL);

        IRBuilder<> Builder(si);
        Builder.CreateAlignedStore(trueVal, ptrOpnd, alignment);

        si->eraseFromParent();
      }
    }
  }

  void createStores(AllocaInst* ai) {
    auto alignment = ai->getAlignment();
    auto* trueVal = getTrueVal();

    IRBuilder<> Builder(ai->getNextNode());
    Builder.CreateAlignedStore(trueVal, ai, alignment);

    /*
    auto& instList = ai->getParent()->getInstList();
    BasicBlock::iterator ii(ai);
    ReplaceInstWithValue(instList, ii, trueVal);
    */
  }

  void addStoreInst(AllocaInst* ai) {
    modifyStores(ai);
    createStores(ai);
  }

  void makeUseNvmTrue(Function& F) {
    for (auto& I : instructions(F)) {
      if (auto* ddi = dyn_cast<DbgDeclareInst>(&I)) {
        auto* var = ddi->getVariable();
        auto varName = var->getName();
        if (var && isUseNvm(varName)) {
          auto* addr = ddi->getAddress();
          if (auto* ai = dyn_cast<AllocaInst>(addr)) {
            addStoreInst(ai);
          }
        }
      }
    }
  }

  void runOptimization() {}

  void makeUseNvmTrue(Module& M) {
    PassBuilder PB;
    FunctionPassManager FPM;
    FunctionAnalysisManager FA;
    PB.registerFunctionAnalyses(FA);

    for (auto& F : M) {
      if (F.isDeclaration() || F.isIntrinsic())
        continue;

      FPM.addPass(SROA());
      FPM.addPass(SCCPPass());
      FPM.addPass(ADCEPass());
      FPM.addPass(SimplifyCFGPass());

      makeUseNvmTrue(F);
      FPM.run(F, FA);
    }
  }

  Module& M;

public:
  SetConstant(Module& M_) : M(M_) { makeUseNvmTrue(M_); }
}

;

} // namespace llvm