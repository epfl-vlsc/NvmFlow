#pragma once
#include "Common.h"
#include "FunctionSet.h"

namespace llvm {

class NameFilter {
  static constexpr const char* flushNames[] = {"_Z10pm_clflushPKv",
                                               "_Z13pm_clflushoptPKv",
                                               "_Z6tx_logPv", "flush_range"};

public:
  static bool isFlush(CallInst* ci) {
    auto* f = ci->getCalledFunction();
    auto n = f->getName();
    for (auto name : flushNames) {
      if (n.equals(name)) {
        return true;
      }
    }
    return false;
  }
};

} // namespace llvm