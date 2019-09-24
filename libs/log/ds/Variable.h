#pragma once
#include "Common.h"
#include "data_util/StructField.h"

namespace llvm {

class Variable {
  using VarSet = std::set<Variable*>;

  StructField* sf;
  StructType* st;
  VarSet writeSet;
  VarSet flushSet;

public:
  Variable(StructField* sf_) : sf(sf_), st(nullptr) { assert(sf); }
  Variable(StructType* st_) : sf(nullptr), st(st_) { assert(st_); }

  bool isField() const { return sf != nullptr; }

  bool isObj() const { return st != nullptr; }

  void addToWriteSet(Variable* var) { writeSet.insert(var); }

  void addToFlushSet(Variable* var) { flushSet.insert(var); }

  auto& getWriteSet() { return writeSet; }

  auto& getFlushSet() { return flushSet; }

  auto* getStructType() {
    if (st)
      return st;
    else
      return sf->getStType();
  }

  void print(raw_ostream& O) const;

  std::string getName() const {
    if (sf)
      return sf->getName();
    else
      return st->getName().str();
  }

  bool operator<(const Variable& X) const {
    return std::tie(sf, st) < std::tie(X.sf, X.st);
  }

  bool operator==(const Variable& X) const { return sf == X.sf && st == X.st; }
};


} // namespace llvm