#pragma once
#include "Common.h"
#include "analysis_util/AliasGroups.h"
#include "data_util/StructField.h"
#include "parser_util/ParsedVariable.h"

namespace llvm {

class Variable {
  Value* localVar;
  Type* type;
  StructField* sf;
  bool locRef;

  bool annotated;
  StringRef localName;

  Variable(Value* localVar_, Type* type_, StructField* sf_, bool locRef_,
           bool annotated_, StringRef localName_)
      : localVar(localVar_), type(type_), sf(sf_), locRef(locRef_),
        annotated(annotated_), localName(localName_) {
    assert(localVar && type);
  }

public:
  static Variable getVariable(ParsedVariable pv, StructField* sf,
                              StringRef localName) {
    // use after passing durability conditions
    auto* localVar = pv.getLocalVar();
    auto* type = pv.getType();
    bool locRef = pv.isLocRef();
    bool annotated = pv.isAnnotated();
    return Variable(localVar, type, sf, locRef, annotated, localName);
  }

  static Variable getSearchVariable(Value* localVar, Type* type,
                                    StructField* sf, bool locRef) {
    // use after passing durability conditions
    return Variable(localVar, type, sf, locRef, false, "");
  }

  bool isField() const { return sf != nullptr; }

  bool isObj() const { return sf == nullptr; }

  bool isAnnotated() const { return isField() && annotated; }

  bool isLocRef() const { return locRef; }

  void print(raw_ostream& O) const;

  auto getName() const {
    std::string name;
    raw_string_ostream typedName(name);

    if (!localName.empty())
      typedName << localName;

    if (sf)
      typedName << "->" << sf->getName();

    return typedName.str();
  }

  bool operator<(const Variable& X) const {
    return std::tie(localVar, type, sf, locRef) <
           std::tie(X.localVar, X.type, X.sf, X.locRef);
  }

  bool operator==(const Variable& X) const {
    return localVar == X.localVar && type == X.type && sf == X.sf &&
           locRef == X.locRef;
  }
};

} // namespace llvm