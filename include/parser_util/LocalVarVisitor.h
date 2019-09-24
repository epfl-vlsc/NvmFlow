#pragma once
#include "Common.h"

#include "llvm/IR/InstVisitor.h"
#include "data_util/NamedFunctions.h"

namespace llvm {

struct LocalVarVisitor : public InstVisitor<LocalVarVisitor, Value*> {
  using BaseVisitor = InstVisitor<LocalVarVisitor, Value*>;

  Value* visitInstruction(Instruction& I) {
    errs() << "instr:" << I.getOpcodeName() << I << "\n";
    report_fatal_error("parse fail - not supported");
    return nullptr;
  }

  Value* visitStore(StoreInst& I) {
    auto* v = I.getPointerOperand();
    return visit(*v);
  }

  Value* visitCastInst(CastInst& I) {
    auto* v = I.getOperand(0);
    return visit(*v);
  }

  Value* visitGetElementPtrInst(GetElementPtrInst& I) {
    auto* v = I.getPointerOperand();
    return visit(*v);
  }

  Value* visitLoadInst(LoadInst& I) {
    auto* v = I.getPointerOperand();
    return visit(*v);
  }

  Value* visitIntrinsicInst(IntrinsicInst& I) {
    auto* v = I.getOperand(0);
    return visit(*v);
  }

  Value* visitCallInst(CallInst& I) {
    if (NameFilter::isVarCall(&I)) {
      auto* v = I.getOperand(0);
      return visit(*v);
    } else {
      return &I;
    }
  }

  Value* visitPHINode(PHINode& I) { return &I; }

  Value* visitAllocaInst(AllocaInst& I) { return &I; }

  Value* visitBinaryOperator(BinaryOperator& I) {
    auto* v = I.getOperand(0);
    return visit(*v);
  }

  Value* visitPtrToIntInst(PtrToIntInst& I) {
    auto* v = I.getPointerOperand();
    return visit(*v);
  }

  Value* visitInvokeInst(InvokeInst& I) { return &I; }

  Value* visit(Value& V) {
    if (auto* a = dyn_cast<Argument>(&V)) {
      return a;
    } else if (auto* c = dyn_cast<Constant>(&V)) {
      return c;
    } else if (auto* i = dyn_cast<Instruction>(&V)) {
      return BaseVisitor::visit(*i);
    }

    errs() << "value:" << V << "\n";
    report_fatal_error("parse fail - not supported");
    return nullptr;
  }
};

} // namespace llvm