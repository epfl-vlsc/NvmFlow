#pragma once
#include "AnnotatedFunctions.h"
#include "Common.h"
#include "FunctionSet.h"

namespace llvm {

// add name to here and respective struct
class NameFilter {
  static constexpr const char* fncNames[] = {"_Z6pfencev", "_Z6vfencev",
                                             "_Z8tx_beginv", "_Z6tx_endv"};

  static constexpr const char* varCalls[] = {
      "_Z8pm_flushPKv",       "_Z13pm_flushfencePKv", "flush_range",
      "pmemobj_tx_add_range", "_Z6tx_logPv",          "llvm.memcpy",
      "llvm.memmove"};

  static constexpr const char* storeFunctions[] = {"llvm.memcpy",
                                                   "llvm.memmove"};

  static constexpr const size_t ElementSize = sizeof(const char*);

public:
  static bool contains(CallInst* ci, const char* const* names, size_t size) {
    auto* f = ci->getCalledFunction();
    if (!f)
      return false;

    auto n = f->getName();
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

  static bool isVarCall(CallInst* ci) {
    return contains(ci, varCalls, sizeof(varCalls) / ElementSize);
  }

  static bool isStoreFunction(CallInst* ci) {
    return contains(ci, storeFunctions, sizeof(storeFunctions) / ElementSize);
  }
};

} // namespace llvm