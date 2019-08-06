#pragma once
#include "Common.h"
#include "StructElement.h"

namespace llvm {

class VarElement {
public:
  enum VarType { Obj, Field, AnnotatedField, None };
  static constexpr const char* VarTypeStr[] = {"Obj", "Field", "AnnotatedField",
                                               "None"};

private:
  Type* objType;
  StructElement* se;
  VarType varType;
  AliasSet* aliasSet;

  std::set<VarElement*> writeSet;
  std::set<VarElement*> flushSet;

public:
  VarElement(Type* objType_, StructElement* se_, VarType varType_,
             AliasSet* aliasSet_)
      : objType(objType_), se(se_), varType(varType_), aliasSet(aliasSet_) {}

  bool isFieldType() const {
    return varType == Field || varType == AnnotatedField;
  }

  bool isObj() const { return varType == Obj; }

  bool isField() const { return varType == Field; }

  bool isAnnotatedField() const { return varType == AnnotatedField; }

  auto getName() const {
    std::string name;
    name.reserve(100);
    name += getTypeName(objType) + " ";
    if (se)
      name += se->getName() + " ";
    name += VarTypeStr[(int)varType];
    return name;
  }

  bool operator<(const VarElement& X) const {
    return objType < X.objType || se < X.se || varType < X.varType ||
           aliasSet < X.aliasSet;
  }

  bool operator==(const VarElement& X) const {
    return objType == X.objType && se == X.se && varType == X.varType &&
           aliasSet == X.aliasSet;
  }
};

} // namespace llvm