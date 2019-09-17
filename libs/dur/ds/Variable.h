#pragma once
#include "Common.h"
#include "analysis_util/AliasGroups.h"
#include "data_util/StructField.h"

namespace llvm {

class Variable {
  Value* localVar;
  Value* aliasVal;
  Type* type;

  StructField* sf;

public:
  Variable(Value* localVar_, Value* aliasVal_, Type* type_, StructField* sf_)
      : localVar(localVar_), aliasVal(aliasVal_), type(type_), sf(sf_) {
    assert(sf);
  }

  bool isField() const { return sf != nullptr; }

  bool isObj() const { return sf == nullptr; }

  void print(raw_ostream& O) const;

  std::string getName() const {
    if (sf)
      return sf->getName();
    else {
      return getTypeName(type);
    }
  }

  bool operator<(const Variable& X) const {
    return std::tie(localVar, aliasVal, type, sf) <
           std::tie(X.localVar, X.aliasVal, X.type, X.sf);
  }

  bool operator==(const Variable& X) const {
    return localVar == X.localVar && aliasVal == X.aliasVal && type == X.type &&
           sf == X.sf;
  }
};

} // namespace llvm