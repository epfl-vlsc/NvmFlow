#pragma once
#include "Common.h"

namespace llvm {

struct AliasGroups {
  using AliasSet = std::vector<Value*>;
  using AliasSets = std::vector<AliasSet>;
  using ValueCat = std::map<Value*, int>;

  static constexpr const int InvalidSetNo = -1;

  AliasSets aliasSets;
  ValueCat valueCat;
  ValueCat localCat;
  AAResults& AAR;

  size_t size() const { return aliasSets.size(); }

  int getAliasSetNo(Value* v) {
    if (valueCat.count(v)) {
      return valueCat[v];
    }
    return InvalidSetNo;
  }

  static bool isInvalidNo(int no) { return no == InvalidSetNo; }

  // skip alias checking for local variables that are on the stack
  void insert(Value* v, Value* lv) {
    if (localCat.count(lv)) {
      int setNo = localCat[lv];
      addToAliasSet(setNo, v);
    } else {
      aliasSets.push_back(AliasSet());
      int setNo = aliasSets.size() - 1;
      localCat[lv] = setNo;
      addToAliasSet(setNo, v);
    }
  }

  void addToAliasSet(int setNo, Value* v) {
    auto& aliasSet = aliasSets[setNo];
    aliasSet.push_back(v);
    valueCat[v] = setNo;
  }

  void insert(Value* v) {
    // todo check
    auto* type = v->getType();
    if (!type->isPointerTy())
      return;

    int setNo = 0;
    for (auto& aliasSet : aliasSets) {
      auto* e = aliasSet[0];
      auto res = AAR.alias(v, e);
      if (res != NoAlias) {
        addToAliasSet(setNo, v);
        return;
      }
      setNo++;
    }

    // create new set
    aliasSets.push_back(AliasSet());
    addToAliasSet(setNo, v);
  }

  void print(raw_ostream& O) const {
    O << "Alias Groups\n";
    O << "------------\n";
    for (auto& [val, setNo] : valueCat) {
      O << "(";
      if (auto* i = dyn_cast<Instruction>(val))
        O << DbgInstr::getSourceLocation(i);
      else
        O << *val;
      O << "," << setNo << ")\n";
    }
  }

  AliasGroups(AAResults& AAR_) : AAR(AAR_) {}
};

} // namespace llvm