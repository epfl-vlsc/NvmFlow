#pragma once
#include "Common.h"
#include "analysis_util/AliasGroups.h"
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

class VarInfo {
  Value* localVar;
  Value* aliasVal;
  Type* type;
  StructField* sf;
  bool locRef;

  bool annotated;
  std::string localName;

  VarInfo(Value* localVar_, Value* aliasVal_, Type* type_, StructField* sf_,
          bool locRef_, bool annotated_, std::string localName_)
      : localVar(localVar_), aliasVal(aliasVal_), type(type_), sf(sf_),
        locRef(locRef_), annotated(annotated_), localName(localName_) {
    assert(localVar && type);
  }

public:
  static VarInfo getVarInfo(ParsedVariable pv, StructField* sf, bool annotated,
                            std::string localName) {
    auto* localVar = pv.getLocalVar();
    auto* aliasVal = pv.getOpndVar();
    auto* type = pv.getType();
    bool locRef = pv.isLocRef();
    return VarInfo(localVar, aliasVal, type, sf, locRef, annotated, localName);
  }

  auto* getAliasValue() { return aliasVal; }

  bool isField() const { return sf != nullptr; }

  bool isObj() const { return sf == nullptr; }

  bool isAnnotated() const { return isField() && annotated; }

  bool isLocRef() const { return locRef; }

  auto getName() const {
    std::string name;
    raw_string_ostream typedName(name);

    if (!localName.empty())
      typedName << localName;

    if (sf)
      typedName << "->" << sf->getName();

    return typedName.str();
  }

  bool operator<(const VarInfo& X) const {
    return std::tie(localVar, aliasVal, type, sf, locRef) <
           std::tie(X.localVar, X.aliasVal, X.type, X.sf, X.locRef);
  }

  bool operator==(const VarInfo& X) const {
    return localVar == X.localVar && aliasVal == X.aliasVal && type == X.type &&
           sf == X.sf && locRef == X.locRef;
  }
};

} // namespace llvm