#pragma once
#include "Common.h"
#include "data_util/StructElement.h"

namespace llvm {

using Variable = StructElement;

class PairVariable {
  Variable* data;
  Variable* valid;

  bool useDcl;

public:
  PairVariable(Variable* data_, Variable* valid_, bool useDcl_)
      : data(data_), valid(valid_), useDcl(useDcl_) {
    assert(data && valid);
  }

  bool isDcl() const { return useDcl; }

  auto* getData() { return data; }

  auto* getValid() { return valid; }

  auto* getPair(Variable* var) {
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
    return data < X.data || valid < X.valid;
  }

  bool operator==(const PairVariable& X) const {
    return data == X.data && valid == X.valid;
  }
};

} // namespace llvm