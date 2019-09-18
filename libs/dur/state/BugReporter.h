#pragma once
#include "BugData.h"
#include "Common.h"
#include "analysis_util/Backtrace.h"
#include "analysis_util/DataflowResults.h"
#include "analysis_util/DfUtil.h"

namespace llvm {

template<typename Globals, typename LatVar, typename LatVal>
class BugReporter {
  using AbstractState = std::map<LatVar, LatVal>;
  
  using FunctionResults = std::map<Value*, AbstractState>;
  using ContextResults = std::map<Context, FunctionResults>;
  using AllResults = DataflowResults<AbstractState>;
  
  using BuggedVars = std::set<Variable*>;
  using SeenContext = std::set<Context>;
  using ContextList = std::vector<Context>;
  using InstrLoc = typename Backtrace::InstrLoc;

  auto getVarName(Variable* var, InstrInfo* ii) {
    std::string name;
    return name;
  }

  bool reportWriteBug(Variable* var, InstrInfo* ii, AbstractState& state) {

    return false;
  }

  void checkWrite(InstrInfo* ii, AbstractState& state) {}

  void checkFlush(InstrInfo* ii, AbstractState& state) {
    
  }

  void checkFinalBugs() {
    auto& state = allResults.getFinalState();
    for (auto& [var, val] : state) {
      if (buggedVars.count(var))
        continue;

      if (!val.isDclFence()) {
        auto mangledName = topFunction->getName();
        auto fncName = globals.dbgInfo.getFunctionName(mangledName).str();
        auto varName = var->getName();
        FinalCommitBug::report(varName, fncName);
        bugNo++;
      }
    }
  }

  template <typename FunctionResults>
  void checkBugs(Instruction* i, Context& context, FunctionResults& results) {
    // get instruction info

    auto* ii = globals.locals.getInstrInfo(i);
    if (!ii)
      return;

    // check if instruction has changed state
    auto* instKey = Traversal::getInstructionKey(i);
    if (!results.count(instKey))
      return;

    // get state
    auto& state = results[instKey];

    // check type of instruction
    switch (ii->getInstrType()) {
    case InstrInfo::WriteInstr: {
      checkWrite(ii, state);
      break;
    }
    case InstrInfo::FlushInstr:
      checkFlush(ii, state);
      break;
    case InstrInfo::FlushFenceInstr:
      checkFlush(ii, state);
      break;
    case InstrInfo::IpInstr: {
      auto* ci = dyn_cast<CallInst>(i);
      auto* f = ci->getCalledFunction();
      Context newContext(context, ci);
      checkBugs(f, newContext);
      contextList.pop_back();
      break;
    }
    default:
      return;
    }
  }

  bool seenContext(Context& context) {
    contextList.push_back(context);
    bool seenCtx = seen.count(context);
    seen.insert(context);
    return seenCtx;
  }

  void checkBugs(Function* f, Context& context) {
    if (seenContext(context))
      return;

    // get function results
    auto& results = allResults.getFunctionResults(context);

    // traverse the dataflow codebase
    for (auto& BB : Traversal::getBlocks(f)) {
      for (auto& I : Traversal::getInstructions(&BB)) {
        checkBugs(&I, context, results);
      }
    }
  }

  void resetStructures(Function* f) {
    topFunction = f;
    buggedVars.clear();
    seen.clear();
    contextList.clear();
    bugNo = 0;
  }

  void reportNumBugs() {
    auto mangledName = topFunction->getName();
    auto fncName = globals.dbgInfo.getFunctionName(mangledName);
    errs() << "Number of bugs in " << fncName << ": " << bugNo << "\n\n\n";
  }

  void reportTitle() {
    auto mangledName = topFunction->getName();
    auto fncName = globals.dbgInfo.getFunctionName(mangledName);
    errs() << "Bugs in " << fncName << "\n";
  }

  // data structures
  Globals& globals;

  // per analysis structures
  AllResults& allResults;

  // bug report structures
  Function* topFunction;
  BuggedVars buggedVars;
  SeenContext seen;
  ContextList contextList;
  int bugNo;

public:
  BugReporter(Globals& globals_, AllResults& allResults_)
      : globals(globals_), allResults(allResults_), topFunction(nullptr),
        bugNo(0) {}

  ~BugReporter() { resetStructures(nullptr); }

  void checkBugs(Function* f) {
    // allResults.print(errs());
    resetStructures(f);

    auto c = Context();
    reportTitle();
    checkBugs(f, c);
    checkFinalBugs();
    reportNumBugs();
  }
};

} // namespace llvm