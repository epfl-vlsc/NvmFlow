#pragma once
#include "Common.h"

namespace llvm {

class SingleVariable {
  static constexpr const int OBJ_ID = -1;

  StructType* st;
  int idx;

public:
  SingleVariable(StructType* st_, int idx_) : st(st_), idx(idx_) {
    assert(idx > -2);
  }

  auto getStType() const { return st; }

  bool isObj() const { return idx == -1; }

  bool isField() const { return idx != -1; }

  std::string getName() const {
    return st->getName().str() + ":" + std::to_string(idx);
  }

  void print(raw_ostream& O) const { O << getName(); }

  bool operator<(const SingleVariable& X) const {
    return st < X.st || idx < X.idx;
  }

  bool operator==(const SingleVariable& X) const {
    return st == X.st && idx == X.idx;
  }
};

class Variable {
  SingleVariable* data;
  SingleVariable* valid;

  bool useDcl;

public:
  Variable(SingleVariable* data_, SingleVariable* valid_, bool useDcl_)
      : data(data_), valid(valid_), useDcl(useDcl_) {
    assert(data && valid);
  }

  bool isDcl() const { return useDcl; }

  auto* getData() { return data; }

  auto* getValid() { return valid; }

  auto* getPair(SingleVariable* sv) {
    assert(sv == data || sv == valid);
    return (sv == data) ? (valid) : data;
  }

  auto* getUsed(bool isData) {
    if (isData) {
      return data;
    } else {
      return valid;
    }
  }

  auto getName() const {
    std::string name;
    name.reserve(100);
    name += "(" + (useDcl) ? "dcl," : "scl,";
    name += data->getName() + "," + valid->getName() + ")";
    return name;
  }

  void print(raw_ostream& O) const { O << getName(); }

  bool operator<(const Variable& X) const {
    return data < X.data || valid < X.valid;
  }

  bool operator==(const Variable& X) const {
    return data == X.data && valid == X.valid;
  }
};

} // namespace llvm