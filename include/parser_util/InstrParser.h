#pragma once
#include "Common.h"
#include "ParsedVariable.h"

#include "LocalVarVisitor.h"

namespace llvm {

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

Type* getPtrElementType(Type* t) {
  assert(t && t->isPointerTy());
  auto* pt = dyn_cast<PointerType>(t);
  return pt->getPointerElementType();
}

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

  static Value* getLocalVar(Instruction* i) {
    assert(i);
    LocalVarVisitor lvv;
    auto* localVar = lvv.visit(*i);

    assert(localVar);
    return localVar;
  }

  static bool isUsed(Instruction* i) {
    if (auto* si = dyn_cast<StoreInst>(i)) {
      return true;
    } else if (auto* ci = dyn_cast<CallInst>(i)) {
      return NameFilter::isVarCall(ci);
    }

    return false;
  }

  static Type* getOpndType(Instruction* i) {
    assert(i);
    if (auto* si = dyn_cast<StoreInst>(i)) {
      return si->getPointerOperandType();
    } else if (auto* ci = dyn_cast<CallInst>(i)) {
      auto* opnd = ci->getArgOperand(0);
      if (auto* castInst = dyn_cast<CastInst>(opnd)) {
        return castInst->getSrcTy();
      } else if (auto* loadInst = dyn_cast<LoadInst>(opnd)) {
        return loadInst->getPointerOperandType();
      } else if (auto* arg = dyn_cast<Argument>(opnd)) {
        return arg->getType();
      } else if (auto* gepi = dyn_cast<GetElementPtrInst>(opnd)) {
        auto* gepiOpnd = gepi->getPointerOperand();
        return gepiOpnd->getType();
      } else if (auto* invokeInst = dyn_cast<InvokeInst>(opnd)) {
        return invokeInst->getType();
      } else if (auto* callInst = dyn_cast<CallInst>(opnd)) {
        return opnd->getType();
      }

      errs() << "opnd: " << *opnd << "\n";
    }

    errs() << "inst: " << *i << "\n";
    report_fatal_error("wrong inst - type");
    return nullptr;
  }

public:
  static std::pair<Value*, Value*> getOpndVar(Instruction* i) {
    assert(i);
    if (auto* si = dyn_cast<StoreInst>(i)) {
      auto* opndLhs = si->getPointerOperand();
      auto* opndRhs = si->getValueOperand();
      return std::pair(opndLhs, opndRhs);
    } else if (auto* ci = dyn_cast<CallInst>(i)) {
      // todo - currently ignore rhs of llvm.memcpy
      auto* opnd = ci->getArgOperand(0);
      return std::pair(opnd, nullptr);
    }

    report_fatal_error("wrong inst - opnd");
    return std::pair(nullptr, nullptr);
  }

  static auto parseInstruction(Instruction* i) {
    if (!isUsed(i))
      return ParsedVariable();

    auto instCat = ParsedVariable::getInstCat(i);
    auto [opndVar, opndRhs] = getOpndVar(i);
    auto* localVar = getLocalVar(i);
    auto* type = getOpndType(i);

    // fill parsed variable----------------------------------
    bool isPtr = false;    ////////
    bool isLocRef = false; //////
    StringRef annotation = EmptyRef;

    // skip cast
    auto* v = stripCasts(opndVar);

    // skip array field
    v = stripAggregate(v);

    // ref-------------------------------------------------
    if (auto* li = dyn_cast<LoadInst>(v)) {
      // use location
      v = li->getPointerOperand();
      isLocRef = true;
      isPtr = true;
    } else {
      // use variable
      isPtr = usesPtr(opndVar);
    }

    // skip cast------------------------------------------
    v = stripCasts(v);

    // check annotation----------------------------------
    if (auto* ii = dyn_cast<IntrinsicInst>(v)) {
      annotation = getAnnotation(ii);
      v = ii->getOperand(0);
    }

    // skip cast------------------------------------------
    v = stripCasts(v);

    // fix type and locref---------------------------------
    if (isa<CallInst>(v) || isa<Argument>(v)) {
      isLocRef = true;
      isPtr = true;
    }

    if (!isLocRef)
      type = getPtrElementType(type);

    // fix type for varRef---------------------------------------------
    if (isa<AllocaInst>(v) || isa<Argument>(v) || isa<CallInst>(v)) {
      // objptr
      return ParsedVariable(opndVar, localVar, type, instCat, isLocRef,
                            opndRhs);
    }
    // check field/objptr
    else if (auto* gepi = dyn_cast<GetElementPtrInst>(v)) {
      // field
      auto [st, idx] = getStructInfo(gepi);

      // disable dynamic offsets
      if (idx < 0)
        return ParsedVariable();

      if (st && isa<StructType>(st)) {
        auto* structType = dyn_cast<StructType>(st);
        return ParsedVariable(opndVar, localVar, type, instCat, isPtr, isLocRef,
                              structType, idx, annotation, opndRhs);
      }
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
}; // namespace llvm

} // namespace llvm