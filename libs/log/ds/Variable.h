#pragma once
#include "Common.h"

namespace llvm {

class Variable {
  StructType* st;
  int idx;

public:
  Variable(StructType* st_, int idx_) : st(st_), idx(idx_) {
    assert(idx > -2);
  }

  auto getStType() const { return st; }

  bool isObj() const { return idx == -1; }

  bool isField() const { return idx != -1; }

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