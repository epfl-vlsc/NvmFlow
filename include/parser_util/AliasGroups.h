#pragma once
#include "Common.h"

namespace llvm {

struct AliasGroupsBase {
  using AliasSet = std::vector<Value*>;
  using AliasSets = std::vector<AliasSet>;
  using ValueCat = std::map<Value*, int>;

  static constexpr const int InvalidSetNo = -1;

  AliasSets aliasSets;
  ValueCat valueCat;
  ValueCat localCat;
  AAResults& AAR;

  size_t size() const { return aliasSets.size(); }

  int getSetNo(Value* v) {
    if (valueCat.count(v)) {
      return valueCat[v];
    }
    return InvalidSetNo;
  }

  bool isValidSet(int setNo) { return setNo != InvalidSetNo; }

  void print(raw_ostream& O) const {
    O << "Alias Groups\n";
    O << "------------\n";
    for (auto& [val, setNo] : valueCat) {
      O << "(";
      if (auto* i = dyn_cast<Instruction>(val))
        O << DbgInstr::getSourceLocation(i) << ",";
      
      O << *val;
      O << "," << setNo << ")\n";
    }
  }

  void addToAliasSet(Value* v, int setNo) {
    auto& aliasSet = aliasSets[setNo];
    aliasSet.push_back(v);
    valueCat[v] = setNo;
  }

  AliasGroupsBase(AAResults& AAR_) : AAR(AAR_) {}

  virtual void insert(Value* v) = 0;
};

struct AliasGroups : public AliasGroupsBase {
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
        addToAliasSet(v, setNo);
        return;
      }
      setNo++;
    }

    // create new set
    aliasSets.push_back(AliasSet());
    addToAliasSet(v, setNo);
  }

  AliasGroups(AAResults& AAR_) : AliasGroupsBase(AAR_) {}
};

struct SparseAliasGroups : public AliasGroupsBase {
  void insert(Value* v) {
    // todo check
    auto* type = v->getType();
    if (!type->isPointerTy())
      return;

    int setNo = 0;
    for (auto& aliasSet : aliasSets) {
      bool addSet = true;
      for (auto* e : aliasSet) {
        auto res = AAR.alias(v, e);
        if (res == NoAlias) {
          addSet = false;
          break;
        }
      }

      if (addSet) {
        addToAliasSet(v, setNo);
        return;
      }

      setNo++;
    }

    // create new set
    aliasSets.push_back(AliasSet());
    addToAliasSet(v, setNo);
  }

  SparseAliasGroups(AAResults& AAR_) : AliasGroupsBase(AAR_) {}
};

} // namespace llvm