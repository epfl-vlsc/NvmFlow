#pragma once
#include "Common.h"
#include "llvm/Transforms/Utils/Local.h"

namespace llvm {

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
    O << "|" << ICStr[(int)ic];
    O << " " << VCStr[(int)vc];
    if (!isUsed()) {
      O << "|";
      return;
    }

    O << " " << RCStr[(int)rt];

    O << " alias:" << *aliasVar;
    O << " local:" << *localVar;

    if (isField())
      O << " " << st->getStructName() << " " << std::to_string(idx);

    if (isAnnotated())
      O << " " << annotation;

    O << "|";
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

  auto* getStructType() {
    assert(st);
    return st;
  }

  auto* getObjElementType() {
    assert(localVar);
    auto* type = localVar->getType();
    assert(type->isPointerTy());
    auto* ptrType = dyn_cast<PointerType>(type);
    auto* objType = ptrType->getPointerElementType();
    assert(type);
    return objType;
  }

  auto getStructInfo() {
    assert(st && idx >= 0);
    return std::pair(st, idx);
  }

  auto getAnnotation() const {
    assert(isAnnotated());
    return annotation;
  }
};

class InstrParser {
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
      if (auto* ci = dyn_cast<CastInst>(v)) {
        v = ci->getOperand(0);
      } else {
        return v;
      }
    }

    report_fatal_error("went back too much");
    return (Value*)nullptr;
  }

  static bool usesPtr(Value* opnd) {
    // assume islocref is false
    auto* type = opnd->getType();
    auto* ptrType = dyn_cast<PointerType>(type);
    assert(ptrType);
    auto* eType = ptrType->getPointerElementType();
    return eType->isPointerTy();
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

    // ref
    if (auto* li = dyn_cast<LoadInst>(v)) {
      // use location
      v = li->getPointerOperand();
      v = skipBackwards(v);
      isLocRef = true;
      isPtr = true;
    } else {
      // use variable
      isPtr = usesPtr(opnd);
    }

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