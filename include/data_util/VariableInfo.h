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

  void print(raw_ostream& O) const {
    O << getName();
  }

  bool operator<(const VariableInfo& X) const { return st < X.st || idx < X.idx; }

  bool operator==(const VariableInfo& X) const { return st == X.st || idx == X.idx; }
};

} // namespace llvm