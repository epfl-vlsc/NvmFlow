#pragma once
#include "Common.h"
#include "analysis_util/DfUtil.h"
namespace llvm {

class Backtrace {
  using ContextList = std::vector<Context>;

  void addToQueue(Function* f) {
    assert(f);
    auto* bb = &f->back();
    addToQueue(bb);
  }

  void addToQueue(Instruction* i) {
    assert(i);
    while (i->getPrevNonDebugInstruction()) {
      i = i->getPrevNonDebugInstruction();
      travQueue.push(i);
    }
    auto* bb = i->getParent();
    for (auto* predBB : Traversal::getPredecessorBlocks(bb)) {
      travQueue.push(predBB);
    }
  }

  void addToQueue(BasicBlock* bb) {
    assert(bb);
    auto* lastInstr = &bb->back();
    addToQueue(lastInstr);
  }

  bool isValidContext(int contextNo) { return contextNo >= 0; }

  auto* getContextFunction(Context& context) {
    if (auto* ci = context.getCallee()) {
      return ci->getCalledFunction();
    }
    return topFunction;
  }

  std::queue<Value*> travQueue;
  Function* topFunction;

public:
  Backtrace(Function* f) : topFunction(f) {}

  template <typename Variable, typename AllResults>
  auto* getValueInstruction(Instruction* curInstr, Variable* curVar,
                            AllResults& allResults, ContextList& contextList) {
    std::vector<Instruction*> instrs;

    // get context
    int contextNo = contextList.size() - 1;
    auto context = contextList[contextNo];

    // get initial state and value
    assert(allResults.inAllResults(context));
    auto results = allResults.getFunctionResults(context);
    auto* instKey = Traversal::getInstructionKey(curInstr);
    assert(results.count(instKey));
    auto& beginState = results[instKey];
    auto& beginVal = beginState[curVar];

    // add the previous instructions in the current BB
    addToQueue(curInstr);

    while (isValidContext(contextNo)) {
      // context traversal

      while (!travQueue.empty()) {
        // bb instr traversal
        auto* v = travQueue.front();
        travQueue.pop();
        if (auto* i = dyn_cast<Instruction>(v)) {
          // i
          auto* instKey = Traversal::getInstructionKey(i);
          if (!results.count(instKey))
            continue;

          auto state = results[instKey];
          auto val = beginState[curVar];

          if (val == beginVal) {
            // same lattice val
            instrs.push_back(i);
          } else {
            // return last same state instr
            return instrs.back();
          }
        } else if (auto* bb = dyn_cast<BasicBlock>(v)) {
          // bb
          addToQueue(bb);
        }
      }

      // get upper context
      contextNo--;
      if (isValidContext(contextNo)) {
        context = contextList[contextNo];
        auto* f = getContextFunction(context);
        addToQueue(f);
        results = allResults.getFunctionResults(context);
      }
    }
    
    if (instrs.empty())
      return curInstr;

    return instrs.back();
  }
};

} // namespace llvm