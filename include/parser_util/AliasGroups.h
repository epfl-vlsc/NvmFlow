#pragma once
#include "Common.h"

#include "ParsedVariable.h"

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

  bool isValidAlias(ParsedVariable& pv) {
    auto* v = pv.getAlias();
    auto aliasSetNo = getAliasSetNo(v);

    return aliasSetNo != InvalidSetNo;
  }

  int getAliasSetNo(ParsedVariable& pv) {
    auto* v = pv.getAlias();
    return getAliasSetNo(v);
  }

  static bool isInvalidNo(int no) { return no == InvalidSetNo; }

  void addToAliasSet(int setNo, Value* v) {
    auto& aliasSet = aliasSets[setNo];
    aliasSet.push_back(v);
    valueCat[v] = setNo;
  }

  void insert(ParsedVariable& pv) {
    auto* v = pv.getAlias();
    insert(v);
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