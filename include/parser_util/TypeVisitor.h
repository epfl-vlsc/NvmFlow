#pragma once
#include "Common.h"

#include "data_util/NameFilter.h"
#include "llvm/IR/InstVisitor.h"

namespace llvm {

struct LhsTypeVisitor : public InstVisitor<LhsTypeVisitor, Type*> {
  using BaseVisitor = InstVisitor<LhsTypeVisitor, Type*>;

  auto visitInstruction(Instruction& I) {
    errs() << "instr:" << I.getOpcodeName() << I << "\n";
    report_fatal_error("lhs type parse fail - not supported");
    return nullptr;
  }

  auto visitStore(StoreInst& I) { return I.getPointerOperandType(); }

  auto visitCastInst(CastInst& I) { return I.getSrcTy(); }

  auto visitGetElementPtrInst(GetElementPtrInst& I) {
    auto* gepiOpnd = I.getPointerOperand();
    return gepiOpnd->getType();
  }

  auto visitLoadInst(LoadInst& I) { return I.getPointerOperandType(); }

  auto visitInvokeInst(InvokeInst& I) { return I.getType(); }

  auto visit(Value& V) {
    if (auto* a = dyn_cast<Argument>(&V)) {
      return a->getType();
    } else if (auto* c = dyn_cast<Constant>(&V)) {
      return c->getType();
    } else if (auto* i = dyn_cast<Instruction>(&V)) {
      return BaseVisitor::visit(*i);
    }

    errs() << "value:" << V << "\n";
    report_fatal_error("lhs type parse fail - not supported");
    return V.getType();
  }

  auto visitCallInst(CallInst& I) {
    auto* opnd = I.getArgOperand(0);
    if (auto* ci = dyn_cast<CallInst>(opnd))
      return opnd->getType();
    return visit(*opnd);
  }
};

struct RhsTypeVisitor : public InstVisitor<RhsTypeVisitor, Type*> {
  using BaseVisitor = InstVisitor<RhsTypeVisitor, Type*>;

  auto visitInstruction(Instruction& I) {
    errs() << "instr:" << I.getOpcodeName() << I << "\n";
    report_fatal_error("rhs type parse fail - not supported");
    return nullptr;
  }

  auto visitStore(StoreInst& I) {
    auto* opnd = I.getValueOperand();
    return opnd->getType();
  }

  auto visitCastInst(CastInst& I) { return I.getSrcTy(); }

  auto visitGetElementPtrInst(GetElementPtrInst& I) {
    return I.getPointerOperandType();
  }

  auto visitLoadInst(LoadInst& I) { return I.getPointerOperandType(); }

  auto visitInvokeInst(InvokeInst& I) { return I.getType(); }

  auto visit(Value& V) {
    if (auto* a = dyn_cast<Argument>(&V)) {
      return a->getType();
    } else if (auto* c = dyn_cast<Constant>(&V)) {
      return c->getType();
    } else if (auto* i = dyn_cast<Instruction>(&V)) {
      return BaseVisitor::visit(*i);
    }

    errs() << "value:" << V << "\n";
    report_fatal_error("rhs type parse fail - not supported");
    return V.getType();
  }

  auto visitCallInst(CallInst& I) {
    if (NameFilter::isStoreFunction(&I)) {
      auto* v = I.getOperand(0);
      return visit(*v);
    }else{
      return I.getType();
    }

    errs() << "instr:" << I << "\n";
    report_fatal_error("rhs type parse fail - not supported");
    return I.getType();
  }
};

struct TypeFinder {
  static auto findTypeRhs(Instruction* i) {
    assert(i);
    RhsTypeVisitor rtv;
    auto res = rtv.visit(*i);
    return res;
  }

  static auto findTypeLhs(Instruction* i) {
    assert(i);
    LhsTypeVisitor ltv;
    auto res = ltv.visit(*i);
    return res;
  }
};

} // namespace llvm