#pragma once
#include "Common.h"

namespace llvm {

template <typename Type> class Worklist {
  std::set<Type> worklist;
  std::list<Type> task;

public:
  void insert(Type e) {
    if (!worklist.count(e)) {
      worklist.insert(e);
      task.push_back(e);
    }
  }

  Type popVal() {
    auto& e = task.front();
    task.pop_front();
    worklist.erase(e);
    return e;
  }

  bool empty() const { return worklist.empty(); }
};

const char* getCIStr(CallInst* ci) {
  if (ci) {
    auto* f = ci->getCalledFunction();
    // data may not be a null terminating ptr
    return f->getName().data();
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
    return std::tie(caller, callee) < std::tie(X.caller, X.callee);
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

  auto* getCaller() { return caller; }

  auto* getCallee() { return callee; }

  bool isTop() const { return !caller && !callee; }

  void print(raw_ostream& O) const { O << getName(); }
};

class Traversal {
public:
  static auto getInstructions(BasicBlock* bb) {
    return iterator_range(bb->begin(), bb->end());
  };

  static auto getBlocks(Function* f) {
    return iterator_range(f->begin(), f->end());
  };

  static Value* getInstructionKey(Instruction* i) { return i; }

  static bool isEntryBlock(BasicBlock* bb) {
    auto* f = bb->getParent();
    auto* entryBlock = &f->front();
    return bb == entryBlock;
  }

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

  static auto getPredecessorBlocks(PHINode* phi) { return phi->blocks(); }

  static auto getPredecessorInstructions(Instruction* i) {
    std::vector<Instruction*> instructions;
    instructions.reserve(20);
    while (i->getPrevNonDebugInstruction()) {
      i = i->getPrevNonDebugInstruction();
      instructions.push_back(i);
    }
    return instructions;
  }

  static auto* getLastBlock(Function* f) { return &f->back(); }
};

} // namespace llvm