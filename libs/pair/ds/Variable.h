#pragma once
#include "Common.h"

namespace llvm {

class Variable {
  using Item = FullStructElement;
  Item* data;
  Item* valid;

  bool useDcl;

public:
  Variable(Item* data_, Item* valid_, bool useDcl_)
      : data(data_), valid(valid_), useDcl(useDcl_) {
    assert(data && valid);
  }

  bool isDcl() const { return useDcl; }

  auto* getData() { return data; }

  auto* getValid() { return valid; }

  auto* getPair(Item* sv) {
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
    static const std::string dclStr = std::string("dcl,");
    static const std::string sclStr = std::string("scl,");

    std::string name;
    name.reserve(100);
    name += "(" + ((useDcl) ? dclStr : sclStr);
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