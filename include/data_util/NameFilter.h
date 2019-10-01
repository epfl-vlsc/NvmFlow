#pragma once
#include "AnnotatedFunctions.h"
#include "Common.h"
#include "FunctionSet.h"

namespace llvm {

// add to var calls and the respective function
class NameFilter {
  static constexpr const char* varCalls[] = {
      "_Z8pm_flushPKv",
      "_Z13pm_flushfencePKv",
      "flush_range",
      "_Z11flush_rangePKvm",
      "pmemobj_tx_add_range",
      "_Z6tx_logPv",
      "llvm.memcpy",
      "llvm.memmove",
      "_ZN7storageL14__pmem_persistEPvmi"};

  static constexpr const char* storeFunctions[] = {"llvm.memcpy",
                                                   "llvm.memmove"};

  static constexpr const char* flushFunctions[] = {"_Z8pm_flushPKv"};

  static constexpr const char* flushfenceFunctions[] = {
      "_Z13pm_flushfencePKv", "flush_range", "_Z11flush_rangePKvm",
      "_ZN7storageL14__pmem_persistEPvmi"};

  static constexpr const char* pfenceFunctions[] = {"_Z6pfencev"};

  static constexpr const char* vfenceFunctions[] = {"_Z6vfencev"};

  static constexpr const char* txBeginFunctions[] = {"_Z8tx_beginv"};

  static constexpr const char* txEndFunctions[] = {"_Z6tx_endv"};

  static constexpr const char* loggingFunctions[] = {"pmemobj_tx_add_range",
                                                     "_Z6tx_logPv"};

  static constexpr const size_t ElementSize = sizeof(const char*);

public:
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
    auto* f = ci->getCalledFunction();
    if (!f)
      return false;

    auto n = f->getName();
    return contains(n, names, size);
  }

  static bool isVarCall(CallInst* ci) {
    return contains(ci, varCalls, sizeof(varCalls) / ElementSize);
  }

  static bool isStoreFunction(CallInst* ci) {
    return contains(ci, storeFunctions, sizeof(storeFunctions) / ElementSize);
  }

  // named functions--------------------------

  static bool isPfenceFunction(StringRef& n) {
    return contains(n, pfenceFunctions, sizeof(pfenceFunctions) / ElementSize);
  }

  static bool isVfenceFunction(StringRef& n) {
    return contains(n, vfenceFunctions, sizeof(vfenceFunctions) / ElementSize);
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
};

} // namespace llvm