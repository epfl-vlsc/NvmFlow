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

class AccessType {
  enum VarType { Obj, Field, AnnotatedField, None };
  static constexpr const char* VarTypeStr[] = {"Obj", "Field", "AnnotatedField",
                                               "None"};
  struct AccessInfo {
    Type* type;
    int idx;
    VarType varType;

    bool isFieldType() const {
      return varType == Field || varType == AnnotatedField;
    }

    bool isObj() const { return varType == Obj; }

    bool isField() const { return varType == Field; }

    bool isAnnotatedField() const { return varType == AnnotatedField; }

    auto getName() const {
      std::string name;
      name.reserve(100);
      name += getTypeName(type) + " ";
      name += std::to_string(idx) + " ";
      name += VarTypeStr[(int)varType];
      return name;
    }
  };

public:
  static AccessInfo getAccessInfo(Instruction* i) {
    assert(isa<LoadInst>(i) || isa<StoreInst>(i) || isa<CastInst>(i));
    if (auto* annotField = getAnnotatedFieldVar(i)) {
      auto [st, idx] = getFieldInfo(annotField);
      return {st, idx, AnnotatedField};
    } else if (auto* field = getFieldVar(i)) {
      auto [st, idx] = getFieldInfo(field);
      return {st, idx, Field};
    } else {
      // assume var
      auto* uncastedVar = getUncasted(i);
      auto* ptrType = uncastedVar->getType();
      assert(ptrType->isPointerTy());
      auto* objType = ptrType->getPointerElementType();
      return {objType, -1, Obj};
    }
  }
};

} // namespace llvm