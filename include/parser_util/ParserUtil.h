#pragma once
#include "Common.h"

namespace llvm {

StoreInst* getStoreFromTxAlloc(Instruction* i) {
  auto* extVal = i->user_back();
  auto* extInst = dyn_cast<Instruction>(extVal);
  assert(isa<ExtractValueInst>(extInst));
  auto* storeVal = extInst->user_back();
  auto* storeInst = dyn_cast<StoreInst>(storeVal);
  assert(storeInst);
  return storeInst;
}

StructType* getToidTxAllocType(Value* v) {
  auto* type = v->getType();
  if (auto* ptrType = dyn_cast<PointerType>(type)) {
    auto* eleType = ptrType->getPointerElementType();
    if (auto* objType = dyn_cast<StructType>(eleType)) {
      return objType;
    }
  }
  return nullptr;
}

bool isTxAllocType(Value* v) {
  if (auto* toidType = getToidTxAllocType(v)) {
    auto toidStName = toidType->getStructName();
    if (toidStName.contains("_toid"))
      return true;
  }
  return false;
}

bool hasToidType(StructType* st, StructType* toid) {
  for (auto* field : st->elements()) {
    if (auto* array = dyn_cast<ArrayType>(field)) {
      auto* eleType = array->getArrayElementType();
      if (eleType == toid)
        return true;
    } else if (field == toid) {
      return true;
    }
  }

  return false;
}

Type* getRealTxAllocType(CallBase* txAlloc, AllocaInst* ai) {
  auto* sizeVal = txAlloc->getOperand(0);
  auto* sizeConst = dyn_cast<ConstantInt>(sizeVal);
  auto size = sizeConst->getZExtValue();

  auto* m = ai->getParent()->getParent()->getParent();
  auto* toidSt = getToidTxAllocType(ai);

  auto& dl = m->getDataLayout();
  for (auto* st : m->getIdentifiedStructTypes()) {
    if (!st->isSized())
      continue;

    auto stSize = dl.getTypeStoreSize(st);
    if (stSize != size)
      continue;

    if (hasToidType(st, toidSt))
      return st->getPointerTo();
  }

  errs() << *ai << " " << toidSt->getStructName() << "\n";
  report_fatal_error("toid unmatched");
  return nullptr;
}

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