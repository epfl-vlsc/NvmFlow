#pragma once
#include "Common.h"
#include "DfUtil.h"

namespace llvm {

template <typename StateMachine> class DataflowAnalysis {
  // df results
  using AbstractState = typename StateMachine::AbstractState;
  using FunctionResults = typename StateMachine::FunctionResults;
  using DataflowResults = typename StateMachine::DataflowResults;

  // context helpers
  using FunctionContext = std::pair<Function*, PlContext>;
  using FunctionContextSet = std::set<FunctionContext>;
  using FunctionContextMap = std::map<FunctionContext, FunctionContextSet>;

  // worklist
  template <typename WorkElement> using Worklist = SetVector<WorkElement>;
  using ContextWorklist = Worklist<FunctionContext>;
  using BlockWorklist = Worklist<BasicBlock*>;

  // data structures
  DataflowResults allResults;
  ContextWorklist contextWork;
  FunctionContextMap callers;
  FunctionContextSet active;

  // top info
  Function* topFunction;
  StateMachine& stateMachine;

  void initTopEntryValues(Function* function, FunctionResults& results) {
    // initialize entry block
    auto* entryBlockKey = Forward::getEntryBlock(function);
    AbstractState& state = results[entryBlockKey];

    // initialize all tracked variables for the entry block
    stateMachine.initLatticeValues(state);
  }

  void initCalleeEntryValues(Function* function, FunctionResults& results) {
    auto* entryBlockKey = Forward::getEntryBlock(function);
    AbstractState& state = results[entryBlockKey];

    // initialize from the function entry state
    auto* functionEntryKey = Forward::getFunctionEntryKey(function);
    state = results[functionEntryKey];
  }

  FunctionResults& getFunctionResults(Function* function,
                                      const PlContext& context) {
    FunctionResults& results = allResults[context];
    auto* functionEntryKey = Forward::getFunctionEntryKey(function);

    if (!results.count(functionEntryKey)) {
      initTopEntryValues(function, results);
    } else {
      initCalleeEntryValues(function, results);
    }

    return results;
  }

  void addBlocksToWorklist(Function* function, BlockWorklist& blockWorkList) {
    for (auto* bb : Forward::getBlocks(function)) {
      blockWorkList.push_back(bb);
    }
  }

  AbstractState mergePrevStates(Value* blockEntryKey,
                                FunctionResults& results) {
    AbstractState mergedState;
    AbstractState& inState = results[blockEntryKey];

    // start with current entry
    mergeInState(mergedState, inState);

    // get all prev blocks
    for (auto* pred_block : Forward::getPredecessorBlocks(blockEntryKey)) {
      auto* blockExitKey = Forward::getExitKey(pred_block);
      if (results.count(blockExitKey)) {
        AbstractState& predecessorState = results[blockExitKey];
        mergeInState(mergedState, predecessorState);
      }
    }

    return mergedState;
  }

  void mergeInState(AbstractState& out, AbstractState& in) {
    for (auto& [inVar, inVal] : in) {
      if (out.count(inVar)) {
        // if var exists meet
        auto& outVal = out[inVar];
        auto meetVal = outVal.meet(inVal);
        out[inVar] = meetVal;
      } else {
        // if var does not exist copy
        out[inVar] = inVal;
      }
    }
  }

  bool analyzeCall(CallInst* ci, AbstractState& callerState, Function* caller,
                   const PlContext& context) {
    PlContext newContext(context, ci);
    Function* callee = ci->getCalledFunction();
    if (!callee)
      return false;

    // prepare function contexts for caller and callee
    FunctionContext toCall = std::pair(callee, newContext);
    FunctionContext toUpdate = std::pair(caller, context);

    // get keys
    auto* calleeEntryKey = Forward::getFunctionEntryKey(callee);
    auto* calleeExitKey = Forward::getFunctionExitKey(callee);

    // get previously computed states in the context
    auto& calleeResults = allResults[newContext];
    AbstractState& calleeEntryState = calleeResults[calleeEntryKey];

    if (active.count(toCall) || callerState == calleeEntryState) {
      return false;
    }

    // update function entry
    calleeEntryState = callerState;

    // do dataflow inter-procedurally
    computeDataflow(callee, newContext);

    // get exit state
    AbstractState& calleeExitState = calleeResults[calleeExitKey];

    // if same result do not update
    if (callerState == calleeExitState) {
      return false;
    }

    // update caller state
    callerState = calleeExitState;
    callers[toCall].insert(toUpdate);

    return true;
  }

  bool applyTransfer(Instruction* i, AbstractState& state) {
    return stateMachine.handleInstruction(i, state);
  }

  bool analyzeStmt(Instruction* i, AbstractState& state, Function* caller,
                   const PlContext& context) {
    if (auto* ci = stateMachine.getCiAnalyzed(i)) {
      return analyzeCall(ci, state, caller, context);
    } else {
      return applyTransfer(i, state);
    }
  }

  void analyzeStmts(BasicBlock* block, AbstractState& state,
                    FunctionResults& results, Function* caller,
                    const PlContext& context) {
    for (auto* i : Forward::getElements(block)) {
      if (analyzeStmt(i, state, caller, context)) {
        auto* stmtKey = Forward::getStmtKey(i);
        results[stmtKey] = state;
      }
    }
  }

  void computeDataflow(Function* function, PlContext& context) {
    active.insert({function, context});

    // initialize results
    FunctionResults& results = getFunctionResults(function, context);

    // initialize worklist
    BlockWorklist blockWorklist;
    addBlocksToWorklist(function, blockWorklist);

    while (!blockWorklist.empty()) {
      auto* block = blockWorklist.pop_back_val();

      auto* blockEntryKey = Forward::getEntryKey(block);
      auto* blockExitKey = Forward::getExitKey(block);

      // get previously computed states
      AbstractState& oldEntryState = results[blockEntryKey];
      AbstractState oldExitState = results[blockExitKey];

      // get current state
      AbstractState state = mergePrevStates(blockEntryKey, results);

      // skip block if same result
      if (state == oldEntryState && !state.empty()) {
        continue;
      }

      // update entry based on predecessors
      results[blockEntryKey] = state;

      // todo go over statements
      analyzeStmts(block, state, results, function, context);

      // skip if state not updated
      if (state == oldExitState) {
        continue;
      }

      // update final state
      results[blockExitKey] = state;

      // do data flow for successive blocks
      for (auto* succBlock : Forward::getSuccessorBlocks(block)) {
        blockWorklist.push_back(succBlock);
      }
    }

    // data flow over all blocks for the function has finished
    // update context information
    // if function changed update all callers
    FunctionResults& oldResults = allResults[context];
    if (oldResults != results) {
      oldResults = results;
      for (const FunctionContext& caller : callers[{function, context}]) {
        contextWork.push_back(caller);
      }
    }

    active.erase({function, context});
  }

  void computeDataflow() {
    while (!contextWork.empty()) {
      auto [function, context] = contextWork.pop_back_val();
      computeDataflow(function, context);
    }
  }

public:
  DataflowAnalysis(Function* function, StateMachine& stateMachine_)
      : topFunction(function), stateMachine(stateMachine_) {
    contextWork.push_back({function, PlContext()});
    computeDataflow();
  }
};

} // namespace clang::ento::nvm