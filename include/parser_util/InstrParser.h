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

Value* stripCastsLoads(Value* v) {
  assert(v);
  while (true) {
    if (auto* ci = dyn_cast<CastInst>(v)) {
      v = ci->getOperand(0);
    } else if (auto* li = dyn_cast<LoadInst>(v)) {
      v = li->getPointerOperand();
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

  static Value* getObj(Instruction* i) { return ObjFinder::findObj(i); }

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

  static Type* getTypeLhs(Instruction* i) {
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

  static std::pair<Type*, bool> getTypeLocRhs(Instruction* i) {
    if (auto* si = dyn_cast<StoreInst>(i)) {
      auto* opnd = si->getValueOperand();
      auto* type = opnd->getType();
      return std::pair(type, false);
    } else if (auto* ci = dyn_cast<CallInst>(i)) {
      if (NameFilter::isStoreFunction(ci)) {
        auto* opnd = ci->getArgOperand(1);
        if (auto* castInst = dyn_cast<CastInst>(opnd)) {
          auto* type = castInst->getSrcTy();
          return std::pair(type, true);
        }

        errs() << "opnd: " << *opnd << "\n";
      }
    }

    errs() << "inst: " << *i << "\n";
    report_fatal_error("wrong inst - type");
    return std::pair<Type*, bool>(nullptr, false);
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

public:
  static auto parseVarLhs(Instruction* i) {
    if (!isUsedLhs(i))
      return ParsedVariable();

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
        isa<LoadInst>(v)) {
      return ParsedVariable(i, obj, opnd, type, isLocRef);
    }

    // field-------------------------------------------
    else if (auto* gepi = dyn_cast<GetElementPtrInst>(v)) {
      // field
      auto [st, idx] = getStructInfo(gepi);

      // disable dynamic offsets
      if (idx < 0)
        return ParsedVariable();

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
      return ParsedVariable();

    auto* obj = getObj(i);
    auto* opnd = getOpnd(i, false);
    auto [type, isLocRef] = getTypeLocRhs(i);
    if (!type->isPointerTy())
      return ParsedVariable();

    //nullptr
    if(auto* cpn = dyn_cast<ConstantPointerNull>(opnd)){
      return ParsedVariable(i, obj, opnd, type);
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
      return ParsedVariable(i, obj, opnd, type, isLocRef);
    }

    // field-------------------------------------------
    else if (auto* gepi = dyn_cast<GetElementPtrInst>(v)) {
      // field
      auto [st, idx] = getStructInfo(gepi);

      // disable dynamic offsets
      if (idx < 0)
        return ParsedVariable();

      if (st && isa<StructType>(st)) {
        auto* structType = dyn_cast<StructType>(st);
        return ParsedVariable(i, obj, opnd, type, isLocRef, structType, idx,
                              annotation);
      }
    }

    // not essential
    return ParsedVariable();
  }
};

} // namespace llvm