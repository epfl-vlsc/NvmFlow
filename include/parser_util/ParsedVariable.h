#pragma once
#include "Common.h"

#include "data_util/NameFilter.h"

namespace llvm {

struct ParsedVariable {
  enum InsCat { StoreIns, CallIns, NoneIns };
  enum VarCat { FieldData, FieldPtr, ObjPtr, NonePtr };
  enum RefCat { VarRef, LocRef }; // access variable or location pointed by it

  static constexpr const char* ICStr[] = {"StoreIns", "CallIns", "NoneIns"};
  static constexpr const char* VCStr[] = {"FieldData", "FieldPtr", "ObjPtr",
                                          "NoneVar"};
  static constexpr const char* RCStr[] = {"VarRef", "LocRef"};

  // use opndVar for alias as well

  InsCat ic;
  Value* opndVar;
  Value* localVar;
  Type* type;
  VarCat vc;
  RefCat rc;

  // field information
  StructType* st;
  int idx;
  StringRef annotation;

  // store info
  Value* opndRhs;

  // none
  ParsedVariable() : ic(NoneIns), vc(NonePtr) {}

  // objptr
  ParsedVariable(Value* opndVar_, Value* localVar_, Type* type_, InsCat ic_,
                 bool isLocRef, Value* opndRhs_)
      : ic(ic_), opndVar(opndVar_), localVar(localVar_), type(type_),
        vc(ObjPtr), rc(isLocRef ? LocRef : VarRef), st(nullptr), idx(-1),
        opndRhs(opndRhs_) {
    assert(opndVar && localVar && type);
  }

  // field
  ParsedVariable(Value* opndVar_, Value* localVar_, Type* type_, InsCat ic_,
                 bool isPtr, bool isLocRef, StructType* st_, int idx_,
                 StringRef annotation_, Value* opndRhs_)
      : ic(ic_), opndVar(opndVar_), localVar(localVar_), type(type_),
        vc(isPtr ? FieldPtr : FieldData), rc(isLocRef ? LocRef : VarRef),
        st(st_), idx(idx_), annotation(annotation_), opndRhs(opndRhs_) {
    assert(opndVar && localVar && type);
    assertField(st, idx);
  }

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

  bool isLocRef() const { return rc == LocRef; }

  bool isPersistentVar() const {
    assert(localVar);
    return NameFilter::isPersistentVar(localVar);;
  }

  auto* getStructType() {
    assert(st);
    return st;
  }

  auto* getObjElementType() {
    assert(localVar);
    auto* oType = localVar->getType();
    assert(type->isPointerTy());
    auto* ptrType = dyn_cast<PointerType>(oType);
    auto* objType = ptrType->getPointerElementType();
    assert(objType);
    return objType;
  }

  StructType* getObjStructType() {
    if (st)
      return st;

    if (auto* ptrType = dyn_cast<PointerType>(type)) {
      auto* objType = ptrType->getPointerElementType();
      assert(objType);

      if (auto* structType = dyn_cast<StructType>(objType))
        return structType;
    }

    return nullptr;
  }

  Type* getObjType() const {
    if (isField()) {
      assert(st);
      return st;
    } else {
      assert(type);
      return type;
    }
  }

  auto* getType() const {
    assert(type);
    return type;
  }

  auto getStructInfo() const {
    assertField(st, idx);
    return std::pair(st, idx);
  }

  auto getAnnotation() const {
    assert(isAnnotated());
    return annotation;
  }

  auto* getLocalVar() {
    assert(localVar);
    return localVar;
  }

  auto* getOpndVar() {
    assert(opndVar);
    return opndVar;
  }

  auto* getRhs() { return opndRhs; }

  void print(raw_ostream& O, bool newline = true) const {
    O << "|(" << ICStr[(int)ic];
    O << "," << VCStr[(int)vc];
    if (!isUsed()) {
      O << ")|";
      if (newline)
        O << "\n";
      return;
    }

    O << "," << RCStr[(int)rc] << ")";

    O << " (type:" << *type << ")";

    O << " (local:" << *localVar << ")";

    if (isField())
      O << " (" << st->getStructName() << "," << std::to_string(idx) << ")";

    if (isAnnotated())
      O << " (" << annotation << ")";

    O << "|";

    if (newline)
      O << "\n";
  }
};

} // namespace llvm