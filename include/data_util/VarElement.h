#pragma once
#include "Common.h"
#include "StructElement.h"
#include "analysis_util/AliasGroups.h"

namespace llvm {

class VarElement {
public:
  enum VarInfo { Obj, Field, AnnotatedField, None };
  static constexpr const char* VarInfoStr[] = {"Obj", "Field", "AnnotatedField",
                                               "None"};

private:
  Type* objType;
  StructElement* se;
  VarInfo varInfo;
  AliasGroup* aliasGroup;
  DILocalVariable* diVar;

  std::set<VarElement*> writeSet;
  std::set<VarElement*> flushSet;

public:
  VarElement(Type* objType_, StructElement* se_, VarInfo varInfo_,
             AliasGroup* aliasGroup_, DILocalVariable* diVar_)
      : objType(objType_), se(se_), varInfo(varInfo_), aliasGroup(aliasGroup_),
        diVar(diVar_) {
    assert(objType && varInfo != None && aliasGroup);
  }

  bool isFieldType() const {
    return varInfo == Field || varInfo == AnnotatedField;
  }

  static auto getVarInfo(StructElement* se, bool isAnnotated) {
    if (isAnnotated) {
      return AnnotatedField;
    } else if (!se || se->isObj()) {
      return Obj;
    } else if (se->isField()) {
      return Field;
    } else {
      errs() << se->getName() << " " << (int)isAnnotated;
      report_fatal_error("wrong se, annotated");
      return None;
    }
  }

  bool isObj() const { return varInfo == Obj; }

  bool isField() const { return varInfo == Field; }

  bool isAnnotatedField() const { return varInfo == AnnotatedField; }

  auto getFieldName() const {
    assert(isField());
    return se->getFieldName();
  }

  auto getName() const {
    std::string name;
    name.reserve(100);
    name += getTypeName(objType) + " ";
    if (se)
      name += se->getName() + " ";
    name += VarInfoStr[(int)varInfo];
    return name;
  }

  bool operator<(const VarElement& X) const {
    return objType < X.objType || se < X.se || varInfo < X.varInfo ||
           aliasGroup < X.aliasGroup || diVar < X.diVar;
  }

  bool operator==(const VarElement& X) const {
    return objType == X.objType && se == X.se && varInfo == X.varInfo &&
           aliasGroup == X.aliasGroup && diVar == X.diVar;
  }
};

} // namespace llvm