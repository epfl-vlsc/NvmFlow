#pragma once
#include "Common.h"

namespace llvm {

auto getUncasted(Value* v) {
  auto* inst = v;

  while (true) {
    if (auto* ci = dyn_cast<CastInst>(inst)) {
      inst = ci->getOperand(0);
    } else {
      return inst;
    }
  }
}

GetElementPtrInst* getGEPI(Value* v) {
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

IntrinsicInst* getII(Value* v) {
  auto* inst = v;

  while (true) {
    if (auto* si = dyn_cast<StoreInst>(inst)) {
      inst = si->getPointerOperand();
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

std::pair<StringRef, bool> isAnnotatedField(IntrinsicInst* ii,
                                            const char* annotation) {
  static const unsigned llvm_ptr_annotation = 186;
  static const StringRef emptyStr = StringRef("");

  if (ii->getIntrinsicID() == llvm_ptr_annotation) {
    if (ConstantExpr* gepi = dyn_cast<ConstantExpr>(ii->getArgOperand(1))) {
      GlobalVariable* AnnotationGL =
          dyn_cast<GlobalVariable>(gepi->getOperand(0));
      StringRef fieldAnnotation =
          dyn_cast<ConstantDataArray>(AnnotationGL->getInitializer())
              ->getAsCString();
      if (fieldAnnotation.startswith(annotation)) {
        return {fieldAnnotation, true};
      }
    }
  }
  return {emptyStr, false};
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

} // namespace llvm