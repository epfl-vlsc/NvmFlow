#pragma once
#include "Common.h"

namespace llvm {

struct AliasGroups {
  using AliasSet = std::vector<Value*>;
  using AliasSets = std::vector<AliasSet>;
  using ValueCat = std::map<Value*, int>;

  AliasSets aliasSets;
  ValueCat valueCat;
  AAResults& AAR;

  void insert(Value* v) {
    //todo check
    auto* type = v->getType();
    if(!type->isPointerTy())
      return;

    int c = 0;
    for (auto& aliasSet : aliasSets) {
      auto* e = aliasSet[0];
      auto res = AAR.alias(v, e);
      if (res != NoAlias) {
        aliasSet.push_back(v);
        valueCat[v] = c;
        return;
      }
      c++;
    }

    // create new set
    aliasSets.push_back(AliasSet());
    aliasSets.at(c).push_back(v);
    valueCat[v] = c;
  }

  void print(raw_ostream& O) const {
    O << "Alias Groups\n";
    O << "------------\n";
    for (auto& [val, no] : valueCat) {
      O << "(" << *val << "," << no << ")\n";
    }
  }

  AliasGroups(AAResults& AAR_) : AAR(AAR_) {}
};

} // namespace llvm