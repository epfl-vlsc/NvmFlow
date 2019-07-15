#pragma once
#include "Common.h"
#include "DfUtil.h"

namespace llvm {

template <typename StateMachine> class DataflowAnalysis {
  // df results
  using AbstractState = typename StateMachine::AbstractState;
  using FunctionResults = std::map<Value*, AbstractState>;
  using DataflowResults = std::map<Context, FunctionResults>;

  // context helpers
  using FunctionContext = std::pair<Function*, Context>;
  using FunctionContextSet = std::set<FunctionContext>;
  using FunctionContextMap = std::map<FunctionContext, FunctionContextSet>;

  // worklist
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
    auto* blockEntryKey = Forward::getBlockEntryKey(function);
    AbstractState& state = results[blockEntryKey];

    // initialize all tracked variables for the entry block
    stateMachine.initLatticeValues(state);
  }

  void initCalleeEntryValues(Function* function, FunctionResults& results) {
    auto* blockEntryKey = Forward::getBlockEntryKey(function);
    AbstractState& state = results[blockEntryKey];

    // initialize from the function entry state
    auto* functionEntryKey = Forward::getFunctionEntryKey(function);
    state = results[functionEntryKey];
  }

  auto& getFunctionResults(Function* function, const Context& context) {
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
    for (auto& BB : Forward::getBlocks(function)) {
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
    for (auto* pred_block : Forward::getPredecessorBlocks(block)) {
      auto* blockExitKey = Forward::getBlockExitKey(pred_block);
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

  bool analyzeCall(CallInst* ci, AbstractState& state, Function* caller,
                   const Context& context) {
    Context newContext(context, ci);
    Function* callee = ci->getCalledFunction();
    if (!callee || callee->isDeclaration())
      return false;

    errs() << "Analyze " << callee->getName() << "\n";

    // prepare function contexts for caller and callee
    FunctionContext toCall = std::pair(callee, newContext);
    FunctionContext toUpdate = std::pair(caller, context);

    // get keys
    auto* calleeEntryKey = Forward::getFunctionEntryKey(callee);
    auto* calleeExitKey = Forward::getFunctionExitKey(callee);

    // get previously computed states in the context
    auto& calleeResults = allResults[newContext];
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
    errs() << "Analyze " << *i << "\n";
    return stateMachine.handleInstruction(i, state);
  }

  bool analyzeStmt(Instruction* i, AbstractState& state, Function* caller,
                   const Context& context) {

    if (stateMachine.isIpaInstruction(i)) {
      auto* ci = dyn_cast<CallInst>(i);
      return analyzeCall(ci, state, caller, context);
    } else {
      return applyTransfer(i, state);
    }
  }

  void analyzeStmts(BasicBlock* block, AbstractState& state,
                    FunctionResults& results, Function* caller,
                    const Context& context) {
    for (auto& I : Forward::getInstructions(block)) {
      auto* i = &I;
      errs() << "lol\n";

      if (analyzeStmt(i, state, caller, context)) {
        auto* instKey = Forward::getInstructionKey(i);
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
      auto* block = blockWorklist.pop_val();

      bool isEntryBlock = Forward::isEntryBlock(block);
      auto* blockEntryKey = Forward::getBlockEntryKey(block);
      auto* blockExitKey = Forward::getBlockExitKey(block);

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
      for (auto* succBlock : Forward::getSuccessorBlocks(block)) {
        blockWorklist.insert(succBlock);
      }
    }

    // data flow over all blocks for the function has finished
    // update context information
    // if function changed update all callers
    FunctionResults& oldResults = allResults[context];
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
      auto [function, context] = contextWork.pop_val();
      computeDataflow(function, context);
    }
  }

public:
  DataflowAnalysis(Function* function, StateMachine& stateMachine_)
      : topFunction(function), stateMachine(stateMachine_) {
    contextWork.insert({function, Context()});
    computeDataflow();
  }

  void print(raw_ostream& O) const {
    O << "---------------------------------\n";
    O << "all results:\n";
    for (auto& [context, functionResults] : allResults) {
      O << context.getName() << "\n";
      for (auto& [location, state] : functionResults) {
        printLocation(location, O);
        for (auto& [latVar, latVal] : state) {
          O << " " << latVar->getName() << " " << latVal.getName() << ",";
        }
        O << "\n";
      }
    }
    O << "---------------------------------\n";
  }
};

} // namespace clang::ento::nvm