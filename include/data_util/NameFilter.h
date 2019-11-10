#pragma once
#include "AnnotatedFunctions.h"
#include "Common.h"
#include "FunctionSet.h"

namespace llvm {

// add to var calls and the respective function
class NameFilter {
public:
  static constexpr const char* PersistentName = "pmemobj_direct";

  static constexpr const char* storeFunctions[] = {"llvm.memcpy",
                                                   "llvm.memmove", "strncpy"};

  static constexpr const char* flushFunctions[] = {"_Z8pm_flushPKv"};

  static constexpr const char* flushfenceFunctions[] = {
      "_Z13pm_flushfencePKv", "flush_range",
      "_ZN7storageL14__pmem_persistEPvmi", "persist_obj", "pmfs_flush_buffer"};

  static constexpr const char* pfenceFunctions[] = {"_Z6pfencev"};

  static constexpr const char* vfenceFunctions[] = {"_Z6vfencev"};

  static constexpr const char* txBeginFunctions[] = {"tx_begin"};

  static constexpr const char* txEndFunctions[] = {"tx_end"};

  static constexpr const char* txLogFunctions[] = {"tx_log"};

  static constexpr const char* txAllocFunctions[] = {"pmemobj_tx_zalloc"};

  static constexpr const char* allocFunctions[] = {"pm_malloc", "pmalloc"};

  static constexpr const size_t ElementSize = sizeof(const char*);

  static bool contains(StringRef n, const char* const* names, size_t size) {
    if (n.empty())
      return false;

    for (size_t i = 0; i < size; ++i) {
      auto* name = names[i];
      if (n.contains(name)) {
        return true;
      }
    }
    return false;
  }

  static bool contains(CallInst* ci, const char* const* names, size_t size) {
    auto* f = getCalledFunction(ci);
    if (!f)
      return false;

    auto n = f->getName();
    return contains(n, names, size);
  }

  static bool equals(StringRef n, const char* const* names, size_t size) {
    if (n.empty())
      return false;

    for (size_t i = 0; i < size; ++i) {
      auto* name = names[i];
      if (n.equals(name)) {
        return true;
      }
    }
    return false;
  }

  static bool equals(CallInst* ci, const char* const* names, size_t size) {
    auto* f = getCalledFunction(ci);
    if (!f)
      return false;

    auto n = f->getName();
    return equals(n, names, size);
  }

  // named functions--------------------------

  static bool isPfenceFunction(StringRef& n) {
    return equals(n, pfenceFunctions, sizeof(pfenceFunctions) / ElementSize);
  }

  static bool isVfenceFunction(StringRef& n) {
    return equals(n, vfenceFunctions, sizeof(vfenceFunctions) / ElementSize);
  }

  static bool isFlushFunction(StringRef& n) {
    return contains(n, flushFunctions, sizeof(flushFunctions) / ElementSize);
  }

  static bool isFlushFenceFunction(StringRef& n) {
    return contains(n, flushfenceFunctions,
                    sizeof(flushfenceFunctions) / ElementSize);
  }

  static bool isStoreFunction(StringRef& n) {
    return contains(n, storeFunctions, sizeof(storeFunctions) / ElementSize);
  }

  static bool isTxEndFunction(StringRef& n) {
    return contains(n, txEndFunctions, sizeof(txEndFunctions) / ElementSize);
  }

  static bool isTxBeginFunction(StringRef& n) {
    return contains(n, txBeginFunctions,
                    sizeof(txBeginFunctions) / ElementSize);
  }

  static bool isTxLogFunction(StringRef& n) {
    return contains(n, txLogFunctions, sizeof(txLogFunctions) / ElementSize);
  }

  static bool isTxAllocFunction(StringRef& n) {
    return contains(n, txAllocFunctions,
                    sizeof(txAllocFunctions) / ElementSize);
  }

  static bool isAllocFunction(StringRef& n) {
    return contains(n, allocFunctions, sizeof(allocFunctions) / ElementSize);
  }

  // parser functions
  static bool isPersistentVar(Value* v) {
    if (auto* cb = dyn_cast<CallBase>(v)) {
      auto* f = getCalledFunction(cb);
      if (f->getName().contains(PersistentName))
        return true;
    }

    return false;
  }

  static bool isAllocFunction(Function& F) {
    auto funcName = F.getName();
    return isAllocFunction(funcName);
  }

  static bool isTxAllocFunction(Function& F) {
    auto funcName = F.getName();
    return isTxAllocFunction(funcName);
  }

  static bool isTxAllocFunction(Value* v) {
    if (auto* cb = dyn_cast<CallBase>(v)) {
      auto* f = getCalledFunction(cb);
      auto funcName = f->getName();
      return isTxAllocFunction(funcName);
    }

    return false;
  }

  static bool isAllocFunction(CallBase* ci) {
    auto* func = getCalledFunction(ci);
    if (!func) {
      return false;
    }
    auto funcName = func->getName();
    return isAllocFunction(funcName);
  }

  static bool isVarCall(CallInst* ci) {
    assert(ci);
    auto* func = getCalledFunction(ci);
    if (!func) {
      return false;
    }

    auto funcName = func->getName();
    bool varCall = isFlushFunction(funcName);
    varCall |= isFlushFenceFunction(funcName);
    varCall |= isStoreFunction(funcName);
    varCall |= isTxLogFunction(funcName);
    varCall |= isTxAllocFunction(funcName);
    return varCall;
  }

  static bool isStoreFunction(CallInst* ci) {
    return contains(ci, storeFunctions, sizeof(storeFunctions) / ElementSize);
  }
};

} // namespace llvm