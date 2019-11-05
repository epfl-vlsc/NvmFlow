#pragma once
#include "Common.h"
#include "data_util/StructField.h"

namespace llvm {

class PairVariable;

class Variable {
  using VarSet = std::set<Variable*>;
  using PairSet = std::set<PairVariable*>;

  StructField* sf;
  StructType* st;
  VarSet writeSet;
  VarSet flushSet;
  PairSet pairs;
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

  void addToPairs(PairVariable* pv) { pairs.insert(pv); }

  int getSetNo() const { return setNo; }

  auto& getWriteSet() { return writeSet; }

  auto& getFlushSet() { return flushSet; }

  auto& getPairs() { return pairs; }

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

  bool operator==(const Variable& X) const { return sf == X.sf && st == X.st && setNo == X.setNo; }
};

class PairVariable {
  Variable* data;
  Variable* valid;

  bool useDcl;

public:
  PairVariable(Variable* data_, Variable* valid_, bool useDcl_)
      : data(data_), valid(valid_), useDcl(useDcl_) {
    assert(data && valid);
    assert(data->getSetNo() == valid->getSetNo());
  }

  bool isDcl() const { return useDcl; }

  auto* getData() { return data; }

  auto* getValid() { return valid; }

  auto getPair() { return std::pair(data, valid); }

  bool isSentinel(Variable* var) const {
    assert(var == data || var == valid);
    return var == valid;
  }

  auto* getOther(Variable* var) {
    assert(var == data || var == valid);
    return (var == data) ? (valid) : data;
  }

  auto* getUsed(bool isData) {
    if (isData) {
      return data;
    } else {
      return valid;
    }
  }

  auto getName() const {
    static const std::string dclStr = std::string("dcl,");
    static const std::string sclStr = std::string("scl,");

    std::string name;
    name.reserve(100);
    name += "(" + ((useDcl) ? dclStr : sclStr);
    name += data->getName() + "," + valid->getName() + ")";
    return name;
  }

  void print(raw_ostream& O) const { O << getName(); }

  bool operator<(const PairVariable& X) const {
    return std::tie(data, valid) < std::tie(X.data, X.valid);
  }

  bool operator==(const PairVariable& X) const {
    return data == X.data && valid == X.valid;
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
  O << ") pairs:(";
  for (auto* pair : pairs) {
    auto* curVar = (Variable*)this;
    O << pair->getOther(curVar)->getName() << ",";
  }
  O << ")";
}

} // namespace llvm