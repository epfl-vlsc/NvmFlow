#pragma once
#include "Common.h"
#include "data_util/NameFilter.h"
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

  // use opndVar for alias as well

  InsCat ic;
  Value* opndVar;
  Value* localVar;
  VarCat vc;
  RefCat rt;
  Type* type;

  // field information
  StructType* st;
  int idx;
  StringRef annotation;

  // none
  ParsedVariable() : ic(NoneIns), vc(NonePtr) {}

  // objptr
  ParsedVariable(Value* opndVar_, Value* localVar_, InsCat ic_, bool isLocRef)
      : ic(ic_), opndVar(opndVar_), localVar(localVar_), vc(ObjPtr),
        rt(isLocRef ? LocRef : VarRef), st(nullptr), idx(-1) {
    assert(opndVar && localVar);
  }

  // field
  ParsedVariable(Value* opndVar_, Value* localVar_, InsCat ic_, bool isPtr,
                 bool isLocRef, StructType* st_, int idx_,
                 StringRef annotation_)
      : ic(ic_), opndVar(opndVar_), localVar(localVar_),
        vc(isPtr ? FieldPtr : FieldData), rt(isLocRef ? LocRef : VarRef),
        st(st_), idx(idx_), annotation(annotation_) {
    assert(opndVar && localVar);
    assert(st && idx >= 0);
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

  Type* getObjType() const {
    if (isField()) {
      assert(st);
      return st;
    } else {
      assert(localVar);
      auto* type = localVar->getType();
      type = stripPointers(type);
      return type;
    }
  }

  auto getStructInfo() const {
    assert(st && idx >= 0);
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

  void print(raw_ostream& O) const {
    O << "|" << ICStr[(int)ic];
    O << " " << VCStr[(int)vc];
    if (!isUsed()) {
      O << "|";
      return;
    }

    O << " " << RCStr[(int)rt];

    auto* type = getObjType();
    O << " ";
    type->print(O);

    O << " opnd:" << *opndVar;
    O << " local:" << *localVar;

    if (isField())
      O << " " << st->getStructName() << " " << std::to_string(idx);

    if (isAnnotated())
      O << " " << annotation;

    O << "|";
  }
};

class InstrParser {
  static constexpr const StringRef EmptyRef;

  static auto getStructInfo(GetElementPtrInst* gepi) {
    assert(gepi);

    Type* type = gepi->getSourceElementType();
    StructType* st = dyn_cast<StructType>(type);

    ConstantInt* index = dyn_cast<ConstantInt>((gepi->idx_end() - 1)->get());

    int idx = -1;
    if (index)
      idx = (int)index->getValue().getZExtValue();

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

  static bool usesPtr(Value* opnd) {
    // assume islocref is false
    auto* type = opnd->getType();
    auto* ptrType = dyn_cast<PointerType>(type);
    assert(ptrType);
    auto* eType = ptrType->getPointerElementType();
    return eType->isPointerTy();
  }

  static Value* getLocalVar(Value* i) {
    auto* v = i;

    assert(v);
    while (true) {
      if (auto* ci = dyn_cast<CastInst>(v)) {
        v = ci->getOperand(0);
      } else if (auto* gepi = dyn_cast<GetElementPtrInst>(v)) {
        v = gepi->getPointerOperand();
      } else if (auto* li = dyn_cast<LoadInst>(v)) {
        v = li->getPointerOperand();
      } else if (auto* si = dyn_cast<StoreInst>(v)) {
        v = si->getPointerOperand();
      } else if (auto* ii = dyn_cast<IntrinsicInst>(v)) {
        return ii->getOperand(0);
      } else if (auto* cai = dyn_cast<CallInst>(v)) {
        if (NameFilter::isFlush(cai)) {
          v = cai->getOperand(0);
        } else {
          return cai;
        }
      } else if (auto* ai = dyn_cast<AllocaInst>(v)) {
        return ai;
      } else if (auto* a = dyn_cast<Argument>(v)) {
        return a;
      } else if (auto* c = dyn_cast<Constant>(v)) {
        return c;
      } else if (auto* phi = dyn_cast<PHINode>(v)) {
        return phi;
      } else {
        break;
      }
    }

    errs() << "v:" << *v << " i:" << *i << "\n";
    report_fatal_error("could not find local variable value");
    return nullptr;
  }

  static bool isUsed(Instruction* i) {
    if (auto* si = dyn_cast<StoreInst>(i)) {
      return true;
    } else if (auto* ci = dyn_cast<CallInst>(i)) {
      return NameFilter::isFlush(ci);
    }

    return false;
  }

public:
  static Value* getOpndVar(Instruction* i, bool lhs = true) {
    if (auto* si = dyn_cast<StoreInst>(i)) {
      auto* opnd = (lhs) ? si->getPointerOperand() : si->getValueOperand();
      return opnd;
    } else if (auto* ci = dyn_cast<CallInst>(i)) {
      auto* opnd = ci->getArgOperand(0);
      return opnd;
    }

    report_fatal_error("opnd has to have value");
    return nullptr;
  }

  static auto parseInstruction(Instruction* i) {
    if (!isUsed(i))
      return ParsedVariable();

    auto instCat = ParsedVariable::getInstCat(i);
    auto* opndVar = getOpndVar(i);
    auto* localVar = getLocalVar(i);

    // fill parsed variable
    bool isPtr = false;    ////////
    bool isLocRef = false; //////
    StringRef annotation = EmptyRef;

    // skip cast
    auto* v = stripCasts(opndVar);

    // ref
    if (auto* li = dyn_cast<LoadInst>(v)) {
      // use location
      v = li->getPointerOperand();
      isLocRef = true;
      isPtr = true;
    } else {
      // use variable
      isPtr = usesPtr(opndVar);
    }

    // skip cast
    v = stripCasts(v);

    // check annotation
    if (auto* ii = dyn_cast<IntrinsicInst>(v)) {
      annotation = getAnnotation(ii);
      v = ii->getOperand(0);
    }

    // skip cast
    v = stripCasts(v);

    // check field/objptr
    if (auto* gepi = dyn_cast<GetElementPtrInst>(v)) {
      // field
      auto [type, idx] = getStructInfo(gepi);
      if (type && isa<StructType>(type)) {
        auto* st = dyn_cast<StructType>(type);
        return ParsedVariable(opndVar, localVar, instCat, isPtr, isLocRef, st,
                              idx, annotation);
      }
    } else if (auto* ai = dyn_cast<AllocaInst>(v)) {
      // objptr
      return ParsedVariable(opndVar, localVar, instCat, isLocRef);
    } else if (auto* a = dyn_cast<Argument>(v)) {
      // objptr
      return ParsedVariable(opndVar, localVar, instCat, isLocRef);
    }

    // not essential
    return ParsedVariable();
  }

  template <typename StructTypes>
  static auto parseInstruction(Instruction* i, StructTypes& sts) {
    auto pv = parseInstruction(i);
    if (!pv.isUsed())
      return pv;

    StructType* st = nullptr;
    if (pv.isObjPtr()) {
      auto* type = pv.getObjElementType();
      while (type && type->isPointerTy()) {
        auto* ptr = dyn_cast<PointerType>(type);
        type = ptr->getPointerElementType();
      }
      st = dyn_cast<StructType>(type);
    } else if (pv.isField()) {
      st = pv.getStructType();
    }

    // check if it is used type
    if (sts.isUsedStructType(st)) {
      return pv;
    }

    return ParsedVariable();
  }
};

} // namespace llvm