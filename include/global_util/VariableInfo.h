#pragma once
#include "Common.h"

namespace llvm {

class VariableInfo {
  StructType* st;
  unsigned idx;

public:
  VariableInfo(StructType* st_, unsigned idx_) : st(st_), idx(idx_) {}

  std::string getName() const {
    return st->getName().str() + ":" + std::to_string(idx);
  }
};

} // namespace llvm