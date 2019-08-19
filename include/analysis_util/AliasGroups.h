#pragma once
#include "Common.h"

namespace llvm {

struct AliasGroup {
  size_t groupNo;
  std::set<Value*> aliasGroup;

  AliasGroup() : groupNo(0) {}

  AliasGroup(Value* v) : groupNo(0) { aliasGroup.insert(v); }

  bool operator<(const AliasGroup& X) const {
    return aliasGroup < X.aliasGroup;
  }

  bool operator==(const AliasGroup& X) const {
    return aliasGroup == X.aliasGroup;
  }

  bool operator!=(const AliasGroup& X) const {
    return aliasGroup != X.aliasGroup;
  }

  auto begin() const { return aliasGroup.begin(); }

  auto end() const { return aliasGroup.end(); }

  auto count(Value* v) const { return aliasGroup.count(v); }

  void insert(Value* v) { aliasGroup.insert(v); }

  auto getName() const {
    assert(groupNo);
    return std::to_string(groupNo);
  }

  void setGroupNo(size_t groupNo_) {
    assert(groupNo_);
    groupNo = groupNo_;
  }
};

class AliasGroups {
  using InstrMap = std::map<Instruction*, Value*>;
  using AliasMap = std::map<Instruction*, AliasGroup*>;

  static constexpr const unsigned llvmPtrAnnotArgs = 4;

  Value* getAliasValue(Value* v) {
    v = v->stripPointerCasts();

    if (auto* ii = dyn_cast<IntrinsicInst>(v)) {
      auto numArgs = ii->getNumArgOperands();
      assert(numArgs == llvmPtrAnnotArgs);

      v = ii->getArgOperand(0);
      v = v->stripPointerCasts();
    }

    return v;
  }

  auto* getPtr(Value* v) {
    auto* vType = v->getType();

    if (vType->isPointerTy()) {
      auto* vPtr = getAliasValue(v);
      auto* vPtrType = vPtr->getType();
      assert(vPtrType->isPointerTy());
      return vPtr;
    }

    return (Value*)nullptr;
  }

  void addAliasGroup(Value* v) {
    AliasGroup ag(v);
    auto [it, _] = aliasGroups.insert(ag);
    auto* agPtr = (AliasGroup*)&*it;
    aliasMap[v] = agPtr;
  }

  AAResults* AAR;
  std::set<AliasGroup> aliasGroups;
  std::map<Value*, AliasGroup*> aliasMap;

public:
  auto begin() { return aliasGroups.begin(); }

  auto end() { return aliasGroups.end(); }

  void add(StoreInst* si) {
    // errs() << *si << "\n";
    auto* siOpnd = si->getPointerOperand();
    if (auto* siPtr = getPtr(siOpnd)) {
      // errs() << "p" << *siPtr << "\n";
      addAliasGroup(siPtr);
    }

    auto* valOpnd = si->getValueOperand();
    if (auto* valPtr = getPtr(valOpnd)) {
      // errs() << "p" << *valPtr << "\n";
      addAliasGroup(valPtr);
    }
  }

  void add(CallInst* ci) {
    // errs() << *ci << "\n";
    auto* ciOpnd = ci->getArgOperand(0);
    if (auto* ciPtr = getPtr(ciOpnd)) {
      // errs() << "p" << *ciPtr << "\n";
      addAliasGroup(ciPtr);
    }
  }

  void add(Instruction* i) {
    assert(isa<StoreInst>(i) || isa<CallInst>(i));
    if (auto* si = dyn_cast<StoreInst>(i)) {
      add(si);
    } else if (auto* ci = dyn_cast<CallInst>(i)) {
      add(ci);
    }
  }

  auto* getAliasGroup(Instruction* i, bool loaded) {
    assert(isa<StoreInst>(i) || isa<CallInst>(i));
    /*
    if (auto* si = dyn_cast<StoreInst>(i)) {
      add(si);
    } else if (auto* ci = dyn_cast<CallInst>(i)) {
      add(ci);
    }
     */
    return (AliasGroup*)nullptr;
  }

  void createGroups(AAResults* AAR_) {
    AAR = AAR_;

    std::set<AliasGroup*> groupsToRemove;
    std::set<Value*> valSeen;
    for (auto [v1, ag1] : aliasMap) {
      valSeen.insert(v1);
      for (auto [v2, ag2] : aliasMap) {
        if (valSeen.count(v2))
          continue;

        auto res = AAR->alias(v1, v2);
        if (res != NoAlias) {
          ag1->insert(v2);
          aliasMap[v2] = ag1;
          groupsToRemove.insert(ag2);
        }
      }
    }

    for (auto* ag : groupsToRemove) {
      aliasGroups.erase(*ag);
    }

    size_t c = 0;
    for (auto& ag : aliasGroups) {
      c++;
      auto* agPtr = (AliasGroup*)&ag;
      agPtr->setGroupNo(c);
    }
  }

  void print(raw_ostream& O) const {
    for (auto ag : aliasGroups) {
      O << "set " << ag.getName() << ":";
      for (auto* v : ag) {
        O << *v;
        if (auto* i = dyn_cast<Instruction>(v)) {
          auto src = DbgInstr::getSourceLocation(i);
          O << "(" << src << ")";
        }
        O << "<=>";
      }
      O << "\n";
    }
  }
};

} // namespace llvm