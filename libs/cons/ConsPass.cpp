#include "ConsPass.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"


#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Transforms/Scalar/ADCE.h"
#include "llvm/Transforms/Scalar/SCCP.h"
#include "llvm/Transforms/Scalar/SROA.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"

namespace llvm {

void ConsPass::print(raw_ostream& OS, const Module* m) const {
  OS << "print bugs here\n";
}

bool isUseNvm(StringRef var) {
  static const char* Str[] = {"useNvm", "use_nvm"};
  for (auto* str : Str) {
    if (var.equals(str))
      return true;
  }
  return false;
}

auto* getTrueVal(Module& M) {
  auto& context = M.getContext();
  auto* boolType = Type::getInt8Ty(context);
  auto* trueVal = ConstantInt::get(boolType, 1);
  return trueVal;
}

void modifyStores(AllocaInst* ai, Module& M) {
  for (auto* user : ai->users()) {
    if (auto* si = dyn_cast<StoreInst>(user)) {
      auto* trueVal = getTrueVal(M);
      auto* ptrOpnd = si->getPointerOperand();
      auto& DL = M.getDataLayout();
      auto alignment = ptrOpnd->getPointerAlignment(DL);

      IRBuilder<> Builder(si);
      Builder.CreateAlignedStore(trueVal, ptrOpnd, alignment);

      si->eraseFromParent();
    }
  }
}

void createStores(AllocaInst* ai, Module& M) {
  auto alignment = ai->getAlignment();
  auto* trueVal = getTrueVal(M);

  IRBuilder<> Builder(ai->getNextNode());
  Builder.CreateAlignedStore(trueVal, ai, alignment);

  /*
  auto& instList = ai->getParent()->getInstList();
  BasicBlock::iterator ii(ai);
  ReplaceInstWithValue(instList, ii, trueVal);
  */
}
void addStoreInst(AllocaInst* ai, Module& M) {
  modifyStores(ai, M);
  createStores(ai, M);
}

bool ConsPass::runOnModule(Module& M) {
  for (auto& F : M) {
    if (F.isDeclaration() || F.isIntrinsic())
      continue;

    for (auto& I : instructions(F)) {
      if (auto* ddi = dyn_cast<DbgDeclareInst>(&I)) {
        auto* var = ddi->getVariable();
        auto varName = var->getName();
        if (var && isUseNvm(varName)) {
          auto* addr = ddi->getAddress();
          if (auto* ai = dyn_cast<AllocaInst>(addr)) {
            addStoreInst(ai, M);
          }
        }
      }
    }
  }

  return true;
}

void ConsPass::getAnalysisUsage(AnalysisUsage& AU) const {}

char ConsPass::ID = 0;
RegisterPass<ConsPass> X("cons", "Cons pass");

} // namespace llvm