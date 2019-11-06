#pragma once
#include "Common.h"
#include "DfUtil.h"
#include "data_util/DbgInfo.h"

namespace llvm {

template <typename StateMachine> class DataflowAnalysis {
  // df results
  using DfResults = typename StateMachine::DfResults;
  using AbstractState = typename DfResults::AbstractState;
  using FunctionResults = typename DfResults::FunctionResults;

  // context helpers
  using FunctionContext = std::pair<Function*, Context>;
  using FunctionContextSet = std::set<FunctionContext>;
  using FunctionContextMap = std::map<FunctionContext, FunctionContextSet>;

  // worklist
  using ContextWorklist = Worklist<FunctionContext>;
  using BlockWorklist = Worklist<BasicBlock*>;

  void initTopEntryValues(Function* function, FunctionResults& results) {
    // initialize entry block
    auto* blockEntryKey = Traversal::getBlockEntryKey(function);
    AbstractState& state = results[blockEntryKey];

    // initialize all tracked variables for the entry block
    stateMachine.initLatticeValues(state);
  }

  void initCalleeEntryValues(Function* function, FunctionResults& results) {
    auto* blockEntryKey = Traversal::getBlockEntryKey(function);
    AbstractState& state = results[blockEntryKey];

    // initialize from the function entry state
    auto* functionEntryKey = Traversal::getFunctionEntryKey(function);
    state = results[functionEntryKey];
  }

  auto& getFunctionResults(Function* function, const Context& context) {
    auto& results = dfResults.getFunctionResults(context);
    auto* functionEntryKey = Traversal::getFunctionEntryKey(function);

    if (!results.count(functionEntryKey)) {
      initTopEntryValues(function, results);
    } else {
      initCalleeEntryValues(function, results);
    }

    return results;
  }

  void addBlocksToWorklist(Function* function, BlockWorklist& blockWorkList) {
    for (auto& BB : Traversal::getBlocks(function)) {
      blockWorkList.insert(&BB);
    }
  }

  AbstractState mergePrevStates(BasicBlock* block, Value* blockEntryKey,
                                FunctionResults& results) {
    AbstractState mergedState;
    AbstractState& inState = results[blockEntryKey];

    // start with current entry
    mergeInState(mergedState, inState);

    // get all prev blocks
    for (auto* pred_block : Traversal::getPredecessorBlocks(block)) {
      auto* blockExitKey = Traversal::getBlockExitKey(pred_block);
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

  bool analyzeCall(CallBase* ci, AbstractState& state, Function* caller,
                   const Context& context) {
    Context newContext(context, ci);
    Function* callee = getCalledFunction(ci);
    if (!callee || callee->isDeclaration())
      return false;

#ifdef DBGMODE
    errs() << "Analyze call " << callee->getName() << "\n";
    /*
    printState(state);
    */
#endif

    // prepare function contexts for caller and callee
    FunctionContext toCall = std::pair(callee, newContext);
    FunctionContext toUpdate = std::pair(caller, context);

    // get keys
    auto* calleeEntryKey = Traversal::getFunctionEntryKey(callee);
    auto* calleeExitKey = Traversal::getFunctionExitKey(callee);

    // get previously computed states in the context
    auto& calleeResults = dfResults.getFunctionResults(newContext);
    AbstractState& calleeEntryState = calleeResults[calleeEntryKey];

    if (active.count(toCall) || state == calleeEntryState) {
      return false;
    }

    // update function entry
    calleeEntryState = state;

    // do dataflow inter-procedurally
    computeDataflow(callee, newContext);

    // get exit state
    AbstractState& calleeExitState = calleeResults[calleeExitKey];

    // if same result do not update
    if (state == calleeExitState) {
      return false;
    }

    // update caller state
    state = calleeExitState;
    callers[toCall].insert(toUpdate);

    return true;
  }

  bool applyTransfer(Instruction* i, AbstractState& state) {
    return stateMachine.handleInstruction(i, state);
  }

  bool analyzeStmt(Instruction* i, AbstractState& state, Function* caller,
                   const Context& context) {

    if (stateMachine.isIpInstruction(i)) {
      auto* ci = dyn_cast<CallBase>(i);
      return analyzeCall(ci, state, caller, context);
    } else {
      return applyTransfer(i, state);
    }
  }

  void analyzeStmts(BasicBlock* block, AbstractState& state,
                    FunctionResults& results, Function* caller,
                    const Context& context) {
    for (auto& I : Traversal::getInstructions(block)) {
      auto* i = &I;

      if (analyzeStmt(i, state, caller, context)) {
        auto* instKey = Traversal::getInstructionKey(i);
        results[instKey] = state;
      }
    }
  }

  void computeDataflow(Function* function, Context& context) {
    active.insert({function, context});

    // initialize results
    FunctionResults& results = getFunctionResults(function, context);

    // initialize worklist
    BlockWorklist blockWorklist;
    addBlocksToWorklist(function, blockWorklist);

    while (!blockWorklist.empty()) {
      auto* block = blockWorklist.popVal();

      bool isEntryBlock = Traversal::isEntryBlock(block);
      auto* blockEntryKey = Traversal::getBlockEntryKey(block);
      auto* blockExitKey = Traversal::getBlockExitKey(block);

      // get previously computed states
      AbstractState& oldEntryState = results[blockEntryKey];
      AbstractState oldExitState = results[blockExitKey];

      // get current state
      AbstractState state = mergePrevStates(block, blockEntryKey, results);

      // skip block if same result
      if (state == oldEntryState && !state.empty() && !isEntryBlock) {
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
      for (auto* succBlock : Traversal::getSuccessorBlocks(block)) {
        blockWorklist.insert(succBlock);
      }
    }

    // data flow over all blocks for the function has finished
    // update context information
    // if function changed update all callers
    auto& oldResults = dfResults.getFunctionResults(context);
    if (oldResults != results) {
      oldResults = results;
      for (const FunctionContext& caller : callers[{function, context}]) {
        contextWork.insert(caller);
      }
    }

    active.erase({function, context});
  }

  void computeDataflow() {
    while (!contextWork.empty()) {
      auto [function, context] = contextWork.popVal();
      computeDataflow(function, context);
    }
  }

  // data structures
  ContextWorklist contextWork;
  FunctionContextMap callers;
  FunctionContextSet active;

  // top info
  DfResults& dfResults;
  StateMachine& stateMachine;

public:
  DataflowAnalysis(Function* function, DfResults& dfResults_,
                   StateMachine& stateMachine_)
      : dfResults(dfResults_), stateMachine(stateMachine_) {
    dfResults.setFunction(function);
    contextWork.insert({function, Context()});
    computeDataflow();
  }
};

} // namespace clang::ento::nvm