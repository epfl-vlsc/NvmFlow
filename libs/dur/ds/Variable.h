#pragma once
#include "Common.h"
#include "parser_util/AliasGroups.h"
#include "data_util/StructField.h"
#include "parser_util/ParsedVariable.h"

namespace llvm {

class Variable {
  int aliasSetNo;

public:
  Variable(int aliasSetNo_) : aliasSetNo(aliasSetNo_) {
    assert(aliasSetNo >= 0);
  }

  auto getName() const { return std::to_string(aliasSetNo); }

  bool operator<(const Variable& X) const { return aliasSetNo < X.aliasSetNo; }

  bool operator==(const Variable& X) const {
    return aliasSetNo == X.aliasSetNo;
  }
};

} // namespace llvm