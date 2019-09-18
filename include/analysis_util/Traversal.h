#pragma once
#include "Common.h"

namespace llvm {
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

  static Value* getInstOrBlockKey(Value* v) { 
    if (auto* i = dyn_cast<Instruction>(v)) {
      return getInstructionKey(i);
    }else if(auto* bb = dyn_cast<BasicBlock>(v)) {
      return getBlockEntryKey(bb);
    }

    report_fatal_error("bb or i");
    return nullptr; 
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
    assert(bb);
    auto* lastInstr = bb->getTerminator();
    assert(lastInstr);
    if (!isa<ReturnInst>(lastInstr)) {
      for (auto& I : instructions(*f)) {
        if (auto* ri = dyn_cast<ReturnInst>(&I)) {
          return ri;
        }
      }
    }

    return lastInstr;
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

  static auto* getPrevInstruction(Instruction* i) {
    return i->getPrevNonDebugInstruction();
  }

  static auto* getLastInstruction(BasicBlock* bb) { return &bb->back(); }

  static auto* getLastBlock(Function* f) {
    auto* lastBB = &f->back();
    assert(lastBB);
    auto* lastInstr = lastBB->getTerminator();
    assert(lastInstr);

    if (!isa<ReturnInst>(lastInstr)) {
      for (auto& I : instructions(*f)) {
        if (auto* ri = dyn_cast<ReturnInst>(&I)) {
          return ri->getParent();
        }
      }
    }
    return lastBB;
  }
};

} // namespace llvm