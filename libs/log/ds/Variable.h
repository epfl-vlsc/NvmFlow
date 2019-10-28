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
  int setNo;

public:
  Variable(StructField* sf_, int setNo_) : sf(sf_), st(nullptr), setNo(setNo_) {
    assert(sf);
  }
  Variable(StructType* st_, int setNo_) : sf(nullptr), st(st_), setNo(setNo_) {
    assert(st_);
  }

  bool isField() const { return sf != nullptr; }

  bool isObj() const { return st != nullptr; }

  void addToWriteSet(Variable* var) { writeSet.insert(var); }

  void addToFlushSet(Variable* var) { flushSet.insert(var); }

  int getSetNo() const { return setNo; }

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
      return sf->getName() + ":" + std::to_string(setNo);
    else
      return st->getName().str() + ":" + std::to_string(setNo);
  }

  bool operator<(const Variable& X) const {
    return std::tie(sf, st, setNo) < std::tie(X.sf, X.st, X.setNo);
  }

  bool operator==(const Variable& X) const {
    return sf == X.sf && st == X.st && setNo == X.setNo;
  }
};

void Variable::print(raw_ostream& O) const {
  O << this->getName();

  O << ": writeSet:(";
  for (auto* var : writeSet) {
    O << var->getName() << ",";
  }

  O << ") flushSet:(";
  for (auto* var : flushSet) {
    O << var->getName() << ",";
  }
  O << ")";
}

} // namespace llvm