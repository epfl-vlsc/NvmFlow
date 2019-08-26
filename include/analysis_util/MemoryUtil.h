#pragma once
#include "Common.h"
#include "llvm/Transforms/Utils/Local.h"

namespace llvm {

AllocaInst* getAllocaInst(Value* v) {
  auto* inst = v;

  while (true) {
    if (auto* si = dyn_cast<StoreInst>(inst)) {
      inst = si->getPointerOperand();
    } else if (auto* ci = dyn_cast<CastInst>(inst)) {
      inst = ci->getOperand(0);
    } else if (auto* gepi = dyn_cast<GetElementPtrInst>(inst)) {
      inst = gepi->getPointerOperand();
    } else if (auto* ii = dyn_cast<IntrinsicInst>(inst)) {
      inst = ii->getArgOperand(0);
    } else if (auto* li = dyn_cast<LoadInst>(inst)) {
      inst = li->getPointerOperand();
    } else if (auto* ai = dyn_cast<AllocaInst>(inst)) {
      return ai;
    } else {
      return nullptr;
    }
  }
}

Function* getParentFunction(Value* v) {
  if (auto* i = dyn_cast<Instruction>(v))
    return i->getParent()->getParent();
  else if (auto* a = dyn_cast<Argument>(v))
    return a->getParent();

  return nullptr;
}

auto* getGepi(Value* v) {
  // use for accessing gepi of non/annot field
  auto* inst = v;

  while (true) {
    if (auto* si = dyn_cast<StoreInst>(inst)) {
      inst = si->getPointerOperand();
    } else if (auto* li = dyn_cast<LoadInst>(inst)) {
      inst = li->getPointerOperand();
    } else if (auto* ci = dyn_cast<CastInst>(inst)) {
      inst = ci->stripPointerCasts();
    } else if (auto* ii = dyn_cast<IntrinsicInst>(inst)) {
      inst = ii->getOperand(0);
    } else if (auto* gepi = dyn_cast<GetElementPtrInst>(inst)) {
      return gepi;
    } else {
      return (GetElementPtrInst*)nullptr;
    }
  }
}

auto* getUncasted(Value* v) {
  auto* inst = v;

  while (true) {
    if (auto* ci = dyn_cast<CastInst>(inst)) {
      inst = ci->getOperand(0);
    } else {
      return dyn_cast<Instruction>(inst);
    }
  }
}

GetElementPtrInst* getGEPI(Value* v) {
  auto* inst = v;

  while (true) {
    if (auto* si = dyn_cast<StoreInst>(inst)) {
      inst = si->getPointerOperand();
    } else if (auto* ci = dyn_cast<CastInst>(inst)) {
      inst = ci->stripPointerCasts();
    } else if (auto* gepi = dyn_cast<GetElementPtrInst>(inst)) {
      return gepi;
    } else {
      return nullptr;
    }
  }
}

IntrinsicInst* getII(Value* v) {
  auto* inst = v;

  while (true) {
    if (auto* si = dyn_cast<StoreInst>(inst)) {
      inst = si->getPointerOperand();
    } else if (auto* li = dyn_cast<LoadInst>(inst)) {
      inst = li->getPointerOperand();
    } else if (auto* ci = dyn_cast<CastInst>(inst)) {
      inst = ci->getOperand(0);
    } else if (auto* gepi = dyn_cast<GetElementPtrInst>(inst)) {
      inst = gepi->getPointerOperand();
    } else if (auto* ii = dyn_cast<IntrinsicInst>(inst)) {
      return ii;
    } else {
      return nullptr;
    }
  }
}

std::pair<bool, StringRef> getAnnotatedField(Value* v, const char* annotation) {
  static const StringRef emptyStr = StringRef("");
  static const unsigned llvm_ptr_annotation = 186;

  auto* ii = getII(v);
  if (!ii) {
    return {false, emptyStr};
  }

  if (ii->getIntrinsicID() == llvm_ptr_annotation) {
    if (ConstantExpr* gepi = dyn_cast<ConstantExpr>(ii->getArgOperand(1))) {
      GlobalVariable* AnnotationGL =
          dyn_cast<GlobalVariable>(gepi->getOperand(0));
      StringRef fieldAnnotation =
          dyn_cast<ConstantDataArray>(AnnotationGL->getInitializer())
              ->getAsCString();
      if (fieldAnnotation.startswith(annotation)) {
        return {true, fieldAnnotation};
      }
    }
  }
  return {false, emptyStr};
}

auto getFieldInfo(IntrinsicInst* ii) {
  Instruction* i = dyn_cast<CastInst>(ii->getArgOperand(0));
  if (i == nullptr && isa<GetElementPtrInst>(ii->getArgOperand(0)))
    i = ii;
  assert(i);

  auto* gepi = dyn_cast<GetElementPtrInst>(i->getOperand(0));
  assert(gepi);
  // assert(gepi->hasAllConstantIndices());

  Type* type = gepi->getSourceElementType();
  assert(type->isStructTy());

  StructType* structType = dyn_cast<StructType>(type);
  assert(structType);

  ConstantInt* index = dyn_cast<ConstantInt>((gepi->idx_end() - 1)->get());
  assert(index);

  int idx = (int)index->getValue().getZExtValue();

  return std::pair(structType, idx);
}

auto getFieldInfo(GetElementPtrInst* gepi) {
  Type* type = gepi->getSourceElementType();
  assert(type->isStructTy());

  StructType* structType = dyn_cast<StructType>(type);
  assert(structType);

  ConstantInt* index = dyn_cast<ConstantInt>((gepi->idx_end() - 1)->get());
  assert(index);

  int idx = (int)index->getValue().getZExtValue();

  return std::pair(structType, idx);
}

auto* getObj(Value* v) {
  auto* uncastedArg0 = v->stripPointerCasts();
  auto* argType = uncastedArg0->getType();
  // must be ptr
  assert(argType->isPointerTy());
  auto* objType = argType->getPointerElementType();

  auto* st = dyn_cast<StructType>(objType);
  return st;
}

auto getAnnotatedField(Value* v) {
  // st == nullptr means invalid
  auto* ii = getII(v);
  StructType* st = nullptr;
  int idx = -1;

  if (!ii) {
    return std::pair(st, idx);
  }

  Instruction* instr = dyn_cast<CastInst>(ii->getArgOperand(0));
  if (instr == nullptr && isa<GetElementPtrInst>(ii->getArgOperand(0)))
    instr = ii;
  assert(instr);

  auto* gepi = dyn_cast<GetElementPtrInst>(instr->getOperand(0));
  assert(gepi);

  Type* type = gepi->getSourceElementType();
  assert(type->isStructTy());

  st = dyn_cast<StructType>(type);
  assert(st);

  ConstantInt* index = dyn_cast<ConstantInt>((gepi->idx_end() - 1)->get());
  assert(index);

  idx = (int)index->getValue().getZExtValue();

  return std::pair(st, idx);
}

auto getField(Value* v) {
  // st == nullptr means invalid
  auto* gepi = getGEPI(v);
  StructType* st = nullptr;
  int idx = -1;

  if (!gepi) {
    return std::pair(st, idx);
  }

  Type* type = gepi->getSourceElementType();
  assert(type->isStructTy());

  st = dyn_cast<StructType>(type);
  assert(st);

  ConstantInt* index = dyn_cast<ConstantInt>((gepi->idx_end() - 1)->get());
  assert(index);

  idx = (int)index->getValue().getZExtValue();

  return std::pair(st, idx);
}

GetElementPtrInst* getFieldVar(Value* v) {
  auto* inst = v;

  while (true) {
    if (auto* si = dyn_cast<StoreInst>(inst)) {
      inst = si->getPointerOperand();
    } else if (auto* ci = dyn_cast<CastInst>(inst)) {
      inst = ci->getOperand(0);
    } else if (auto* gepi = dyn_cast<GetElementPtrInst>(inst)) {
      return gepi;
    } else {
      return nullptr;
    }
  }
}

IntrinsicInst* getAnnotatedFieldVar(Value* v) {
  auto* inst = v;

  while (true) {
    if (auto* si = dyn_cast<StoreInst>(inst)) {
      inst = si->getPointerOperand();
    } else if (auto* li = dyn_cast<LoadInst>(inst)) {
      inst = li->getPointerOperand();
    } else if (auto* ci = dyn_cast<CastInst>(inst)) {
      inst = ci->getOperand(0);
    } else if (auto* gepi = dyn_cast<GetElementPtrInst>(inst)) {
      inst = gepi->getPointerOperand();
    } else if (auto* ii = dyn_cast<IntrinsicInst>(inst)) {
      return ii;
    } else {
      return nullptr;
    }
  }
}

class StoreParser{

};

class FlushParser{


};

} // namespace llvm