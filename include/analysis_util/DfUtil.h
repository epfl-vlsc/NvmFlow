#pragma once
#include "Common.h"

namespace llvm {



const char* getCIStr(CallInst* ci) {
  if (ci) {
    // data may not be a null terminating ptr
    return ci->getName().data();
  } else {
    return "None";
  }
}

class Context {
  CallInst* caller;
  CallInst* callee;

public:
  Context() : caller(nullptr), callee(nullptr) {}
  Context(const Context& X, CallInst* Callee) {
    caller = X.callee;
    callee = Callee;
  }

  bool operator<(const Context& X) const {
    return (caller < X.caller || callee < X.callee);
  }

  bool operator==(const Context& X) const {
    return (caller == X.caller && callee == X.callee);
  }

  bool operator!=(const Context& X) const {
    return (caller != X.caller || callee != X.callee);
  }

  auto getName() const {
    auto name = std::string("Context:(") + getCIStr(caller) + "," +
                getCIStr(callee) + ")";
    return name;
  }

  void print(raw_ostream& O) const { O << getName(); }
};

class Forward {
public:
  static auto getInstructions(BasicBlock* bb) {
    return iterator_range(bb->begin(), bb->end());
  };

  static auto getBlocks(Function* f) {
    return iterator_range(f->begin(), f->end());
  };

  static Value* getInstructionKey(Instruction* i) { return i; }

  static Value* getBlockEntryKey(BasicBlock* bb) { return bb; }

  static Value* getBlockExitKey(BasicBlock* bb) { return bb->getTerminator(); }

  static Value* getBlockEntryKey(Function* f) {
    auto* bb = &f->front();
    return bb;
  }

  static Value* getFunctionEntryKey(Function* f) { return f; }

  static Value* getFunctionExitKey(Function* f) {
    auto* bb = &f->back();
    return bb->getTerminator();
  }

  static auto getSuccessorBlocks(BasicBlock* bb) { return successors(bb); }

  static auto getPredecessorBlocks(BasicBlock* bb) { return predecessors(bb); }
};

} // namespace llvm