#pragma once
#include "Common.h"
#include "analysis_util/DfUtil.h"
namespace llvm {

class Backtrace {
public:
  using ContextList = std::vector<Context>;
  enum InstrLoc { Changed, SameFirst, SameLast };

private:
  void addToQueue(Function* f) {
    if (seenInContext.count(f))
      return;

    assert(f);
    seenInContext.insert(f);
    auto* bb = Traversal::getLastBlock(f);
    addToQueue(bb);
  }

  void addToQueue(Instruction* i) {
    if (seenInContext.count(i))
      return;

    assert(i);
    seenInContext.insert(i);
    while (Traversal::getPrevInstruction(i)) {
      i = Traversal::getPrevInstruction(i);
      travQueue.push(i);
    }
    auto* bb = i->getParent();
    for (auto* predBB : Traversal::getPredecessorBlocks(bb)) {
      travQueue.push(predBB);
    }
  }

  void addToQueue(BasicBlock* bb) {
    if (seenInContext.count(bb))
      return;

    assert(bb);
    seenInContext.insert(bb);
    auto* lastInstr = Traversal::getLastInstruction(bb);
    addToQueue(lastInstr);
  }

  bool isValidContext(int contextNo) { return contextNo >= 0; }

  auto* getContextFunction(Context& context) {
    if (auto* ci = context.getCallee()) {
      return ci->getCalledFunction();
    }
    return topFunction;
  }

  Value* getKey(Value* v, Instruction* curInstr, InstrLoc loc) {
    if (loc == Changed)
      return v;
    else if (values.empty()) {
      return curInstr;
    } else {
      assert(loc == SameFirst);
      return values.back();
    }
  }

  Value* getKey(Value* curInstr, InstrLoc loc) {
    if (values.empty()) {
      return curInstr;
    } else {
      assert(loc == SameFirst);
      return values.back();
    }
  }

  std::set<Value*> seenInContext;
  std::queue<Value*> travQueue;
  std::vector<Value*> values;
  Function* topFunction;

public:
  Backtrace(Function* f) : topFunction(f) {}

  template <typename Variable, typename AllResults, typename EqualityFn>
  Value* getKey(Instruction* curInstr, Variable* curVar, AllResults& allResults,
                ContextList& contextList, EqualityFn eq, InstrLoc loc) {
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

    // add the previous instructions in the current BB
    addToQueue(curInstr);

    while (isValidContext(contextNo)) {
      // context traversal
      while (!travQueue.empty()) {
        // bb instr traversal
        auto* v = travQueue.front();
        travQueue.pop();

        // add bb instructions
        if (auto* bb = dyn_cast<BasicBlock>(v)) {
          addToQueue(bb);
          continue;
        }

        // get key
        auto* key = Traversal::getInstOrBlockKey(v);
        if (!results.count(key))
          continue;

        auto state = results[key];
        auto val = state[curVar];

        if (eq(val, beginVal)) {
          // same lattice val
          if (loc == SameLast)
            return v;

          values.push_back(v);
        } else {
          // return last same state instr
          return getKey(v, curInstr, loc);
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

    return getKey(curInstr, loc);
  }
}; // namespace llvm

} // namespace llvm