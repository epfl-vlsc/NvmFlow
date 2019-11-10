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

Type* getRealTxAllocType(AllocaInst* ai) {
  auto* m = ai->getParent()->getParent()->getParent();
  auto* toidSt = getToidTxAllocType(ai);

  for (auto* st : m->getIdentifiedStructTypes()) {
    auto numElements = st->getNumElements();
    if(numElements != 3)
      continue;

    auto* possibleToId = st->getElementType(2);
    if(possibleToId == toidSt){
      return st->getPointerTo();
    }
      
  }

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