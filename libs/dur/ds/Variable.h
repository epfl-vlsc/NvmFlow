#pragma once
#include "Common.h"
#include "analysis_util/AliasGroups.h"
#include "data_util/StructField.h"

namespace llvm {


class Variable{

};



class SingleVariable {
  using VarSet = std::set<Variable*>;

  StructField* sf;
  Type* type;
  AliasGroup* ag;
  bool isLoc;
  VarSet writeSet;
  VarSet flushSet;

public:
  Variable(StructField* sf_, AliasGroup* ag_, bool isLoc_)
      : sf(sf_), type(nullptr), ag(ag_), isLoc(isLoc_) {
    assert(sf && ag_);
  }
  Variable(Type* type_, AliasGroup* ag_, bool isLoc_)
      : sf(nullptr), type(type_), ag(ag_), isLoc(isLoc_) {
    assert(type_ && ag_);
  }

  bool isField() const { return sf != nullptr; }

  bool isObj() const { return type != nullptr; }

  void addToWriteSet(Variable* var) { writeSet.insert(var); }

  void addToFlushSet(Variable* var) { flushSet.insert(var); }

  auto& getWriteSet() { return writeSet; }

  auto& getFlushSet() { return flushSet; }

  void print(raw_ostream& O) const;

  std::string getName() const {
    if (sf)
      return sf->getName();
    else {
      return getTypeName(type);
    }
  }

  bool operator<(const Variable& X) const {
    return std::tie(sf, st) < std::tie(X.sf, X.st);
  }

  bool operator==(const Variable& X) const { return sf == X.sf && st == X.st; }
};

} // namespace llvm