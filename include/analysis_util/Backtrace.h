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

  auto* getInstr(Instruction* curInstr) {
    if (instrs.empty())
      return curInstr;

    return instrs.back();
  }

  std::queue<Value*> travQueue;
  std::vector<Instruction*> instrs;
  Function* topFunction;

public:
  Backtrace(Function* f) : topFunction(f) {}

  template <typename Variable, typename AllResults, typename EqualityFn>
  auto* getValueInstruction(Instruction* curInstr, Variable* curVar,
                            AllResults& allResults, ContextList& contextList,
                            EqualityFn eq) {

    // get context
    int contextNo = contextList.size() - 1;
    auto context = contextList[contextNo];

    // get initial state and value
    assert(allResults.inAllResults(context));
    auto results = allResults.getFunctionResults(context);
    auto* instKey = Traversal::getInstructionKey(curInstr);
    assert(results.count(instKey));
    auto beginState = results[instKey];
    auto beginVal = beginState[curVar];

    errs() << "states" << DbgInstr::getSourceLocation(curInstr) << "\n";
    printState(beginState);

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
          auto* iKey = Traversal::getInstructionKey(i);
          if (!results.count(iKey))
            continue;

          auto state = results[iKey];
          auto val = state[curVar];

          errs() << "test " << val.getName() << *i << "\n";
          if (eq(val, beginVal)) {
            // same lattice val
            errs() << "add " << val.getName() << *i << "\n";
            instrs.push_back(i);
          } else {
            // return last same state instr
            return getInstr(curInstr);
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

    return getInstr(curInstr);
  }
};

} // namespace llvm