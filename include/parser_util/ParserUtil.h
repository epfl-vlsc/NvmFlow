#pragma once
#include "Common.h"

namespace llvm {

Value* stripCasts(Value* v) {
  assert(v);
  while (true) {
    if (auto* ci = dyn_cast<CastInst>(v)) {
      v = ci->getOperand(0);
    } else {
      break;
    }
  }

  return v;
}

Value* stripCastsLoads(Value* v) {
  assert(v);
  while (true) {
    if (auto* ci = dyn_cast<CastInst>(v)) {
      v = ci->getOperand(0);
    } else if (auto* li = dyn_cast<LoadInst>(v)) {
      v = li->getPointerOperand();
    } else {
      break;
    }
  }

  return v;
}

Value* stripAggregate(Value* v) {
  assert(v);
  if (auto* gepi = dyn_cast<GetElementPtrInst>(v)) {
    auto* type = gepi->getSourceElementType();
    if (type->isArrayTy()) {
      v = gepi->getOperand(0);
    }
  }

  return v;
}

Type* stripPointers(Type* t) {
  assert(t);
  while (t && t->isPointerTy()) {
    auto* pt = dyn_cast<PointerType>(t);
    return pt->getPointerElementType();
    t = pt;
  }
  return t;
}

Type* getPtrElementType(Type* t) {
  assert(t && t->isPointerTy());
  auto* pt = dyn_cast<PointerType>(t);
  return pt->getPointerElementType();
}

} // namespace llvm