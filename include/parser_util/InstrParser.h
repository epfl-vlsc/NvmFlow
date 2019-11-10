#pragma once
#include "Common.h"
#include "ParsedVariable.h"
#include "ParserUtil.h"

#include "LocalVarVisitor.h"
#include "TypeVisitor.h"

namespace llvm {

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

  static Value* getObj(Instruction* i) { return ObjFinder::findObj(i); }

  static Value* getObjRhs(Value* v) {
    if (auto* i = dyn_cast<Instruction>(v)) {
      return ObjFinder::findObj(i);
    }
    return v;
  }

  static bool isUsedLhs(Instruction* i) {
    if (auto* si = dyn_cast<StoreInst>(i)) {
      return true;
    } else if (auto* ci = dyn_cast<CallInst>(i)) {
      return NameFilter::isVarCall(ci);
    }

    return false;
  }

  static bool isUsedRhs(Instruction* i) {
    if (auto* si = dyn_cast<StoreInst>(i)) {
      return true;
    } else if (auto* ci = dyn_cast<CallInst>(i)) {
      return NameFilter::isStoreFunction(ci);
    }

    return false;
  }

  static Type* getTypeLhs(Instruction* i) { return TypeFinder::findTypeLhs(i); }

  static Type* getTypeRhs(Instruction* i) { return TypeFinder::findTypeRhs(i); }

  static bool getLocRhs(Instruction* i) {
    if (auto* si = dyn_cast<StoreInst>(i)) {
      return false;
    } else if (auto* ci = dyn_cast<CallInst>(i)) {
      if (NameFilter::isStoreFunction(ci)) {
        return true;
      }
    }

    errs() << "instr:" << *i << "\n";
    report_fatal_error("rhs loc parse fail - not supported");
    return false;
  }

  static Value* getOpnd(Instruction* i, bool lhs = true) {
    assert(i);
    if (auto* si = dyn_cast<StoreInst>(i)) {
      if (lhs)
        return si->getPointerOperand();
      else
        return si->getValueOperand();
    } else if (auto* ci = dyn_cast<CallInst>(i)) {
      // memcpy rhs
      if (!lhs && NameFilter::isStoreFunction(ci))
        return ci->getArgOperand(1);

      return ci->getArgOperand(0);
    }

    report_fatal_error("wrong inst - opnd");
    return nullptr;
  }

  static auto parseTxAlloc(Instruction* i) {
    auto* extVal = i->user_back();
    auto* extInst = dyn_cast<Instruction>(extVal);
    assert(isa<ExtractValueInst>(extInst));
    auto* storeVal = extInst->user_back();
    auto* storeInst = dyn_cast<Instruction>(storeVal);
    LocalVarVisitor lvv;
    auto* allocVal = lvv.visit(*storeInst);
    auto* obj = dyn_cast<AllocaInst>(allocVal);
    assert(obj);
    auto* type = obj->getType();
    return ParsedVariable(i, obj, i, type, false);
  }

public:
  static auto parseVarLhs(Instruction* i) {
    if (!isUsedLhs(i))
      return ParsedVariable();

    // check for tx_alloc
    if (NameFilter::isTxAllocFunction(i))
      return parseTxAlloc(i);

    // var infos
    auto* obj = getObj(i);
    auto* opnd = getOpnd(i);
    auto* type = getTypeLhs(i);

    // fill parsed variable----------------------------------
    bool isLocRef = false; //////
    StringRef annotation = EmptyRef;

    // skip cast
    auto* v = stripCasts(opnd);

    // skip array field
    v = stripAggregate(v);

    // ref-------------------------------------------------
    if (auto* li = dyn_cast<LoadInst>(v)) {
      // use location
      v = li->getPointerOperand();
      isLocRef = true;
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
    }

    if (!isLocRef)
      type = getPtrElementType(type);

    // obj---------------------------------------------
    if (isa<AllocaInst>(v) || isa<Argument>(v) || isa<CallInst>(v) ||
        isa<LoadInst>(v) || isa<Constant>(v)) {
      return ParsedVariable(i, obj, opnd, type, isLocRef);
    }

    // field-------------------------------------------
    else if (auto* gepi = dyn_cast<GetElementPtrInst>(v)) {
      // field
      auto [st, idx] = getStructInfo(gepi);

      // disable dynamic offsets
      if (idx < 0) {
        // fix type
        type = obj->getType();
        return ParsedVariable(i, obj, opnd, type, isLocRef);
      }

      if (st && isa<StructType>(st)) {
        auto* structType = dyn_cast<StructType>(st);
        return ParsedVariable(i, obj, opnd, type, isLocRef, structType, idx,
                              annotation);
      }
    }

    // not essential
    return ParsedVariable();
  }

  static auto parseVarRhs(Instruction* i) {
    if (!isUsedRhs(i))
      return ParsedVariable(false);

    auto* opnd = getOpnd(i, false);
    auto* obj = getObjRhs(opnd);
    auto* type = getTypeRhs(i);
    auto isLocRef = getLocRhs(i);

    // not interested if not pointer
    if (!type->isPointerTy())
      return ParsedVariable(false);

    if (auto* cons = dyn_cast<Constant>(opnd)) {
      // nullptr
      if (cons->isNullValue())
        return ParsedVariable(i, obj, opnd, type);
    } else if (!obj) {
      // if cannot find object return
      return ParsedVariable(false);
    }

    // fill parsed variable------------------------------
    StringRef annotation = EmptyRef;

    // skip cast
    auto* v = stripCastsLoads(opnd);

    // skip array field----------------------------------
    v = stripAggregate(v);

    // skip cast------------------------------------------
    v = stripCastsLoads(v);

    // check annotation----------------------------------
    if (auto* ii = dyn_cast<IntrinsicInst>(v)) {
      annotation = getAnnotation(ii);
      v = ii->getOperand(0);
    }

    // skip cast------------------------------------------
    v = stripCastsLoads(v);

    // obj---------------------------------------------
    if (isa<AllocaInst>(v) || isa<Argument>(v) || isa<CallInst>(v) ||
        isa<LoadInst>(v)) {
      return ParsedVariable(i, obj, opnd, type, isLocRef, false);
    }

    // field-------------------------------------------
    else if (auto* gepi = dyn_cast<GetElementPtrInst>(v)) {
      // field
      auto [st, idx] = getStructInfo(gepi);

      // disable dynamic offsets
      if (idx < 0)
        return ParsedVariable(false);

      if (st && isa<StructType>(st)) {
        auto* structType = dyn_cast<StructType>(st);
        return ParsedVariable(i, obj, opnd, type, isLocRef, structType, idx,
                              annotation, false);
      }
    }

    // not essential
    return ParsedVariable(false);
  }

  static auto parseEmpty() {
    // not essential
    return ParsedVariable();
  }
};

} // namespace llvm