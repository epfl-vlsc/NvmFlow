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

auto getGepiInfo(GetElementPtrInst* gepi) {
  assert(gepi);

  Type* type = gepi->getSourceElementType();
  assert(type->isStructTy());

  StructType* st = dyn_cast<StructType>(type);
  assert(st);

  ConstantInt* index = dyn_cast<ConstantInt>((gepi->idx_end() - 1)->get());
  assert(index);

  int idx = (int)index->getValue().getZExtValue();
  assert(idx >= 0);

  return std::pair(st, idx);
}

auto getAnnotationRef(IntrinsicInst* ii) {
  static const StringRef emptyStr = StringRef("");
  static const unsigned llvm_ptr_annotation = 186;

  assert(ii);

  if (ii->getIntrinsicID() == llvm_ptr_annotation) {
    if (ConstantExpr* gepi = dyn_cast<ConstantExpr>(ii->getArgOperand(1))) {
      GlobalVariable* AnnotationGL =
          dyn_cast<GlobalVariable>(gepi->getOperand(0));
      StringRef fieldAnnotation =
          dyn_cast<ConstantDataArray>(AnnotationGL->getInitializer())
              ->getAsCString();
      return fieldAnnotation;
    }
  }
  return emptyStr;
}

struct ParsedVariable {
  enum InsCat { StoreIns, CallIns, NoneIns };
  enum VarCat { FieldData, FieldPtr, ObjPtr, NonePtr };
  enum RefCat { VarRef, LocRef }; // access variable or location pointed by it

  static constexpr const char* ICStr[] = {"StoreIns", "CallIns", "NoneIns"};
  static constexpr const char* VCStr[] = {"FieldData", "FieldPtr", "ObjPtr",
                                          "NoneVar"};
  static constexpr const char* RCStr[] = {"VarRef", "LocRef"};

  Value* aliasVar;
  Value* localVar;
  InsCat ic;
  VarCat vc;
  RefCat rt;
  StructType* st;
  int idx;
  StringRef annotation;

  // none
  ParsedVariable() : ic(NoneIns), vc(NonePtr) {}

  // objptr
  ParsedVariable(Value* aliasVar_, Value* localVar_, InsCat ic_, bool isLocRef)
      : aliasVar(aliasVar_), localVar(localVar_), ic(ic_), vc(ObjPtr),
        rt(isLocRef ? LocRef : VarRef) {
    assert(aliasVar && localVar);
  }

  // field
  ParsedVariable(Value* aliasVar_, Value* localVar_, InsCat ic_, bool isPtr,
                 bool isLocRef, StructType* st_, int idx_,
                 StringRef annotation_)
      : aliasVar(aliasVar_), localVar(localVar_), ic(ic_),
        vc(isPtr ? FieldPtr : FieldData), rt(isLocRef ? LocRef : VarRef),
        st(st_), idx(idx_), annotation(annotation_) {
    assert(aliasVar && localVar);
    assert(st && idx >= 0);
  }

  void print(raw_ostream& O) const {
    O << ICStr[(int)ic];
    O << " " << VCStr[(int)vc];
    if (!isUsed())
      return;

    O << " " << RCStr[(int)rt];

    O << " alias:" << *aliasVar;
    O << " local:" << *localVar;

    if (isField())
      O << " " << st->getStructName() << " " << std::to_string(idx);

    if (isAnnotated())
      O << " " << annotation;
  }

  static bool isUsed(InsCat ic_) { return ic_ != NoneIns; }

  static auto getInstCat(Instruction* i) {
    if (auto* si = dyn_cast<StoreInst>(i)) {
      return StoreIns;
    } else if (auto* ci = dyn_cast<CallInst>(i)) {
      return CallIns;
    }

    return NoneIns;
  }

  bool isUsed() const { return ic != NoneIns; }

  bool isAnnotated() const { return !annotation.empty(); }

  bool isFieldData() const { return vc == FieldData; }

  bool isFieldPtr() const { return vc == FieldPtr; }

  bool isObjPtr() const { return vc == ObjPtr; }

  bool isField() const { return isFieldData() || isFieldPtr(); }

  auto getAnnotation() const {
    assert(isAnnotated());
    return annotation;
  }
};

class InstructionParser {
  static auto getLocalVar(GetElementPtrInst* gepi) {
    assert(gepi);
    return gepi->getPointerOperand();
  }

  static auto getStructInfo(GetElementPtrInst* gepi) {
    assert(gepi);

    Type* type = gepi->getSourceElementType();
    assert(type->isStructTy());

    StructType* st = dyn_cast<StructType>(type);
    assert(st);

    ConstantInt* index = dyn_cast<ConstantInt>((gepi->idx_end() - 1)->get());
    assert(index);

    int idx = (int)index->getValue().getZExtValue();

    return std::pair(st, idx);
  }

  static auto getAnnotation(IntrinsicInst* ii) {
    static const StringRef emptyStr = StringRef("");
    static const unsigned llvm_ptr_annotation = 186;

    assert(ii);

    if (ii->getIntrinsicID() == llvm_ptr_annotation) {
      if (ConstantExpr* gepi = dyn_cast<ConstantExpr>(ii->getArgOperand(1))) {
        GlobalVariable* AnnotationGL =
            dyn_cast<GlobalVariable>(gepi->getOperand(0));
        StringRef fieldAnnotation =
            dyn_cast<ConstantDataArray>(AnnotationGL->getInitializer())
                ->getAsCString();
        return fieldAnnotation;
      }
    }
    return emptyStr;
  }

  static auto* getOpnd(Instruction* i) {
    if (auto* si = dyn_cast<StoreInst>(i)) {
      auto* opnd = si->getPointerOperand();
      return opnd;
    } else if (auto* ci = dyn_cast<CallInst>(i)) {
      auto* opnd = ci->getArgOperand(0);
      return opnd;
    }

    report_fatal_error("opnd has to have value");
    return (Value*)nullptr;
  }

  static auto* skipBackwards(Value* v) {
    assert(v);
    while (true) {
      if (auto* li = dyn_cast<LoadInst>(v)) {
        v = li->getPointerOperand();
      } else if (auto* ci = dyn_cast<CastInst>(v)) {
        v = ci->stripPointerCasts();
      } else {
        return v;
      }
    }
    return (Value*)nullptr;
  }

public:
  static auto parseInstruction(Instruction* i) {
    static const StringRef emptyStr = StringRef("");
    auto instCat = ParsedVariable::getInstCat(i);
    if (!ParsedVariable::isUsed(instCat))
      return ParsedVariable();

    // get opnd of interest
    Value* opnd = getOpnd(i);

    // fill parsed variable
    bool isPtr = false;    ////////
    bool isLocRef = false; //////
    StringRef annotation = emptyStr;

    // get rid of pointers
    auto* v = skipBackwards(opnd);

    // check annotation
    if (auto* ii = dyn_cast<IntrinsicInst>(v)) {
      annotation = getAnnotation(ii);
      v = ii->getOperand(0);
      v = skipBackwards(v);
    }

    // check field/objptr
    if (auto* gepi = dyn_cast<GetElementPtrInst>(v)) {
      // field
      auto* localVar = getLocalVar(gepi);
      auto [st, idx] = getStructInfo(gepi);

      return ParsedVariable(opnd, localVar, instCat, isPtr, isLocRef, st, idx,
                            annotation);
    } else if (auto* ai = dyn_cast<AllocaInst>(v)) {
      // objptr
      auto* localVar = ai;

      return ParsedVariable(opnd, localVar, instCat, isLocRef);
    }

    return ParsedVariable();
  }
};

} // namespace llvm