#pragma once
#include "Common.h"

namespace llvm {

class Variable {
  StructType* st;
  int idx;

public:
  Variable(StructType* st_, unsigned idx_) : st(st_), idx(idx_) {
    assert(idx > -2);
  }

  auto getType() const { return st; }

  bool useObj() const { return idx == -1; }

  std::string getName() const {
    return st->getName().str() + ":" + std::to_string(idx);
  }

  void print(raw_ostream& O) const { O << getName(); }

  bool operator<(const Variable& X) const { return st < X.st || idx < X.idx; }

  bool operator==(const Variable& X) const {
    return st == X.st && idx == X.idx;
  }
};

} // namespace llvm