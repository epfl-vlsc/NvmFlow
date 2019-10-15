#pragma once
#include "Common.h"

#include "data_util/NameFilter.h"

namespace llvm {

struct ParsedVariable {
  enum InsCat { StoreIns, CallIns, NoneIns };
  enum VarCat { FieldVar, ObjVar, NullVar, NoneVar };
  enum RefCat { VarRef, LocRef }; // access variable or location pointed by it

  static constexpr const char* ICStr[] = {"StoreIns", "CallIns", "None"};
  static constexpr const char* VCStr[] = {"Field", "Obj", "Null" "None"};
  static constexpr const char* RCStr[] = {"Var", "Loc"};

  // use obj for alias as well
  Value* obj;
  Value* opnd;
  Type* type;

  // info enums
  InsCat ic;
  VarCat vc;
  RefCat rc;

  // optional information
  StructType* st;
  int idx;
  StringRef ann;

  // none
  ParsedVariable() : ic(NoneIns), vc(NoneVar) {}

  // objptr
  ParsedVariable(Instruction* i, Value* obj_, Value* opnd_, Type* type_,
                 bool isLocRef)
      : obj(obj_), opnd(opnd_), type(type_), ic(getInstCat(i)), vc(ObjVar),
        rc(isLocRef ? LocRef : VarRef), st(nullptr), idx(-1) {
    assert(obj && opnd && type);
    auto* objType = obj->getType();
    assert(objType->isPointerTy());
  }

  // field
  ParsedVariable(Instruction* i, Value* obj_, Value* opnd_, Type* type_,
                 bool isLocRef, StructType* st_, int idx_, StringRef ann_)
      : obj(obj_), opnd(opnd_), type(type_), ic(getInstCat(i)), vc(FieldVar),
        rc(isLocRef ? LocRef : VarRef), st(st_), idx(idx_), ann(ann_) {
    assert(obj && opnd && type);
    assertField(st, idx);
  }

  // nullptr
  ParsedVariable(Instruction* i)
      : obj(nullptr), opnd(nullptr), type(nullptr), ic(getInstCat(i)), vc(NullVar),
        rc(LocRef), st(nullptr), idx(-1) {
  }

  InsCat getInstCat(Instruction* i) {
    if (auto* si = dyn_cast<StoreInst>(i)) {
      return StoreIns;
    } else if (auto* ci = dyn_cast<CallInst>(i)) {
      return CallIns;
    }

    return NoneIns;
  }

  bool isUsed() const { return ic != NoneIns; }

  bool isAnnotated() const { return !ann.empty(); }

  bool isField() const { return vc == FieldVar; }

  bool isObj() const { return vc == ObjVar; }

  bool isNull() const { return vc == NullVar; }

  bool isPtr() const { return type->isPointerTy(); }

  bool isVarRef() const { return rc == VarRef; }

  bool isLocRef() const { return rc == LocRef; }

  bool isStoreInst() const { return StoreIns == ic; }

  bool isCallInst() const { return CallIns == ic; }

  bool isPersistentVar() const {
    assert(obj);
    return NameFilter::isPersistentVar(obj);
  }

  auto* getObjElementType() {
    assert(obj);
    auto* oPtrType = obj->getType();
    assert(oPtrType->isPointerTy());
    auto* ptrType = dyn_cast<PointerType>(oPtrType);
    auto* objType = ptrType->getPointerElementType();
    assert(objType);
    return objType;
  }

  // todo a lot of obj finding functions, unify

  Type* getObjType() const {
    if (isField()) {
      assert(st);
      return st;
    } else {
      assert(type);
      return type;
    }
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

  auto* getStructType() {
    assert(st);
    return st;
  }

  Type* getFieldElementType() {
    if (auto* ptrType = dyn_cast<PointerType>(type)) {
      auto* objType = ptrType->getPointerElementType();
      assert(objType);
      return objType;
    }

    return type;
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
    return ann;
  }

  auto* getObj() {
    assert(obj);
    return obj;
  }

  auto* getOpnd() {
    assert(opnd);
    return opnd;
  }

  void print(raw_ostream& O, bool newline = true) const {
    if (!isUsed()) {
      O << "|Not used|";
      if (newline)
        O << "\n";
      return;
    }

    // info enums
    O << "|(" << ICStr[(int)ic] << "," << VCStr[(int)vc] << ","
      << RCStr[(int)rc];
    if (isPtr())
      O << ",Ptr";
    O << ")";

    // obj part
    O << " (obj:" << *obj << ")";
    O << " (type:" << *type << ")";

    // optional part
    if (isField())
      O << " (" << st->getStructName() << "," << std::to_string(idx) << ")";
    if (isAnnotated())
      O << " (" << ann << ")";

    O << "|";
    if (newline)
      O << "\n";
  }
};

} // namespace llvm