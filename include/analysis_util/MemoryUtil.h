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

std::pair<bool, StringRef> getAnnotatedField(Instruction* i,
                                             const char* annotation) {
  static const StringRef emptyStr = StringRef("");
  static const unsigned llvm_ptr_annotation = 186;

  auto* ii = getII(i);
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

auto getField(Instruction* i) {
  // st == nullptr means invalid
  auto* gepi = getGEPI(i);
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
/*
class AccessType {
  enum VarInfo { Obj, Field, AnnotatedField, None };
  static constexpr const char* VarInfoStr[] = {"Obj", "Field", "AnnotatedField",
                                               "None"};
  struct AccessInfo {
    Type* type;
    int idx;
    VarInfo varInfo;

    bool isFieldType() const {
      return varInfo == Field || varInfo == AnnotatedField;
    }

    bool isObj() const { return varInfo == Obj; }

    bool isNone() const { return varInfo == None; }

    bool isField() const { return varInfo == Field; }

    bool isAnnotatedField() const { return varInfo == AnnotatedField; }

    auto getName() const {
      std::string name;
      name.reserve(100);
      name += getTypeName(type) + " ";
      name += std::to_string(idx) + " ";
      name += VarInfoStr[(int)varInfo];
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
      if (auto* si = dyn_cast<StoreInst>(i)) {
        auto* opnd = si->getPointerOperand();
        auto* uncastedVar = getUncasted(opnd);
        auto* ptrType = uncastedVar->getType();
        assert(ptrType->isPointerTy());
        auto* storeType = ptrType->getPointerElementType();
        if (storeType->isPointerTy()) {
          auto* objType = storeType->getPointerElementType();
          return {objType, -1, Obj};
        } else {
          return {nullptr, -1, None};
        }
      } else {
        auto* uncastedVar = getUncasted(i);
        auto* ptrType = uncastedVar->getType();
        assert(ptrType->isPointerTy());
        auto* objType = ptrType->getPointerElementType();
        return {objType, -1, Obj};
      }
    }
  }

  static AccessInfo getLoadedVar(StoreInst* si) {
    auto* val = si->getValueOperand();
    errs() << *val << "\n";
    return {nullptr, -1, None};


        if (auto* annotField = getAnnotatedFieldVar(i)) {
          auto [st, idx] = getFieldInfo(annotField);
          return {st, idx, AnnotatedField};
        } else if (auto* field = getFieldVar(i)) {
          auto [st, idx] = getFieldInfo(field);
          return {st, idx, Field};
        } else {
          // assume var
          if (auto* si = dyn_cast<StoreInst>(i)) {
            auto* opnd = si->getPointerOperand();
            auto* uncastedVar = getUncasted(opnd);
            auto* ptrType = uncastedVar->getType();
            assert(ptrType->isPointerTy());
            auto* storeType = ptrType->getPointerElementType();
            if (storeType->isPointerTy()) {
              auto* objType = storeType->getPointerElementType();
              return {objType, -1, Obj};
            } else {
              return {nullptr, -1, None};
            }
          } else {
            auto* uncastedVar = getUncasted(i);
            auto* ptrType = uncastedVar->getType();
            assert(ptrType->isPointerTy());
            auto* objType = ptrType->getPointerElementType();
            return {objType, -1, Obj};
          }
        }

} // namespace llvm

static auto* getAliasSet(AliasSetTracker* ast, Instruction* i) {
  auto* instr = getUncasted(i);
  if (!isa<LoadInst>(instr) && !isa<StoreInst>(instr)) {
    return (AliasSet*)nullptr;
  }

  auto* type = instr->getType();
  if (type->isPointerTy()) {
    auto memLoc = MemoryLocation::get(instr);
    auto* aliasSet = &(ast->getAliasSetFor(memLoc));
    return aliasSet;
  }

  return (AliasSet*)nullptr;
}
}
;

*/

} // namespace llvm