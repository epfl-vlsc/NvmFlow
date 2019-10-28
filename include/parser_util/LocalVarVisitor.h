#pragma once
#include "Common.h"

#include "data_util/NameFilter.h"
#include "llvm/IR/InstVisitor.h"

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

  Value* visitSExtInst(SExtInst& I) {
    auto* v = I.getOperand(0);
    return visit(*v);
  }

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

struct ObjFinder {
  static bool checkValidObj(Value* obj) {
    bool isValidObj = isa<CallInst>(obj) || isa<InvokeInst>(obj) ||
                      isa<AllocaInst>(obj) || isa<PHINode>(obj) ||
                      isa<Argument>(obj) || isa<GlobalVariable>(obj) ||
                      isa<Constant>(obj);
    if (!isValidObj) {
      errs() << "obj is not valid: " << *obj
             << " value id:" << obj->getValueID() << "\n";
    }

    return isValidObj;
  }

  static Value* findObj(Instruction* i) {
    assert(i);
    LocalVarVisitor lvv;
    auto* obj = lvv.visit(*i);

    assert(obj && checkValidObj(obj));
    return obj;
  }

  static Value* findPersist(Value* v) {
    assert(v);
    LocalVarVisitor lvv;
    auto* obj = lvv.visit(*v);

    assert(obj && checkValidObj(obj));
    return obj;
  }
};

} // namespace llvm