#pragma once
#include "Common.h"

namespace llvm {

class AliasSets {
  struct AliasSet {
    std::set<Instruction*> aliasSet;

    AliasSet() {}
    AliasSet(Instruction* i) { aliasSet.insert(i); }

    bool operator<(const AliasSet& X) const { return aliasSet < X.aliasSet; }

    bool operator==(const AliasSet& X) const { return aliasSet == X.aliasSet; }

    bool operator!=(const AliasSet& X) const { return aliasSet != X.aliasSet; }

    auto begin() const { return aliasSet.begin(); }

    auto end() const { return aliasSet.end(); }

    auto count(Instruction* i) const { return aliasSet.count(i); }

    void insert(Instruction* i) { aliasSet.insert(i); }
  };

  using Instructions = std::set<Instruction*>;
  using InstrMap = std::map<Instruction*, Instruction*>;
  using AliasMap = std::map<Instruction*, AliasSet*>;

  auto* getAliasInstr(Value* v) {
    if (auto* ii = dyn_cast<IntrinsicInst>(v)) {
      auto num = ii->getNumArgOperands();
      errs() << num << "\n";
      auto* gepi = ii->getArgOperand(0);
      gepi = gepi->stripPointerCasts();
      return dyn_cast<Instruction>(gepi);
    }
    return dyn_cast<Instruction>(v);
  }

  void fillInstrMap(InstrMap& instrMap) {
    for (auto* i : instructions) {
      if (auto* si = dyn_cast<StoreInst>(i)) {
        auto* ptrOpnd = si->getPointerOperand();

        auto* ptrInstr = getAliasInstr(ptrOpnd);

        auto* valOpnd = si->getValueOperand();

        auto* valInstr = getAliasInstr(valOpnd);

        errs() << *valOpnd << " " << *valOpnd->stripPointerCasts() << " "
               << getAliasInstr(valOpnd) << "\n";

        auto res = AAR.alias(valOpnd, ptrInstr);
        errs() << res << "\n";

        instrMap[si] = ptrInstr;
      } else if (auto* ci = dyn_cast<CallInst>(i)) {
        auto* arg0Opnd = ci->getArgOperand(0);

        auto* arg0Instr = getAliasInstr(arg0Opnd);

        instrMap[ci] = arg0Instr;
      }
    }
  }

  void addToSet(Instruction* i1, Instruction* i2, AliasResult res) {
    if (res == NoAlias)
      return;

    auto* as = aliasMap[i1];
    bool isMerged = as->count(i2);
    if (isMerged)
      return;

    auto* as2 = aliasMap[i2];
    aliasSets.erase(*as2);

    as->insert(i2);
    aliasMap[i2] = as;
  }

  void formAliasSets(InstrMap& instrMap) {
    for (auto& [i1, alias1] : instrMap) {
      for (auto& [i2, alias2] : instrMap) {
        auto res = AAR.alias(alias1, alias2);
        addToSet(i1, i2, res);
      }
    }
  }

  void initAliasSets(InstrMap& instrMap) {
    for (auto& [i, _] : instrMap) {
      errs() << *i << DbgInstr::getSourceLocation(i) << "\n";
      auto [asit, __] = aliasSets.emplace(i);
      auto* as = (AliasSet*)&*asit;
      aliasMap[i] = as;
    }
  }

  void createAllSets() {
    InstrMap instrMap;
    fillInstrMap(instrMap);
    initAliasSets(instrMap);
    formAliasSets(instrMap);
  }

  void addI(Instruction* i) {
    instructions.insert(i);
    aliasSets.emplace(i);
  }

  AAResults& AAR;
  Instructions instructions;
  std::set<AliasSet> aliasSets;
  AliasMap aliasMap;

public:
  AliasSets(AAResults& AAR_) : AAR(AAR_) {}

  void add(StoreInst* si) { addI(si); }

  void add(CallInst* ci) { addI(ci); }

  void createSets() { createAllSets(); }

  void print(raw_ostream& O) {
    int c = 0;

    for (auto as : aliasSets) {
      O << "set " << c << ":";
      for (auto* i : as) {
        auto src = DbgInstr::getSourceLocation(i);
        if (!src.empty())
          O << DbgInstr::getSourceLocation(i) << ",";
        else
          O << *i << ",";
      }
      O << "\n";
      c++;
    }
  }
};

} // namespace llvm