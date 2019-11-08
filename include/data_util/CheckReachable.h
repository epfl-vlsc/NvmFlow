#pragma once
#include "Common.h"

namespace llvm {

template <typename Globals> class CheckReachable {
  using FunctionSet = std::unordered_set<const Function*>;

  bool blockReachesBlock(BasicBlock* fromBB, BasicBlock* toBB) const {
    return isPotentiallyReachable(fromBB, toBB);
  }

  bool blockReachesFncCall(BasicBlock* fromBB, Function* toF) const {
    // todo
    auto* fromF = fromBB->getParent();
    for (User* U : toF->users()) {
      if (CallBase* cb = dyn_cast<CallBase>(U)) {
        auto* callToFFromBB = cb->getParent();
        auto* callToFFromF = callToFFromBB->getParent();
        if (callToFFromF == fromF)
          return blockReachesBlock(fromBB, callToFFromBB);
      }
      return false;
    }
  }

  Globals& globals;

public:
  CheckReachable(Globals& globals_) : globals(globals_) {}

  bool fncCallsFnc(Function* fromF, Function* toF) {
    return globals.functions.funcCallsFunc(fromF, toF);
  }

  bool instReachesInst(Instruction* fromInst, Instruction* toInst) {
    BasicBlock* fromBB = fromInst->getParent();
    BasicBlock* toBB = toInst->getParent();

    Function* fromF = fromBB->getParent();
    Function* toF = toBB->getParent();

    if (blockReachesBlock(fromBB, toBB))
      return true;
    else if (fromF != toF && fncCallsFnc(fromF, toF) &&
             blockReachesFncCall(fromBB, toF))
      return true;

    return false;
  }

  void print(raw_ostream& O) const {}
};

} // namespace llvm