#pragma once
#include "BugData.h"
#include "Common.h"
#include "FlowTypes.h"
#include "analysis_util/Backtrace.h"
#include "analysis_util/DataflowResults.h"
#include "analysis_util/DfUtil.h"
#include "ds/Globals.h"

namespace llvm {

class BugReporter {
  using AllResults = DataflowResults<AbstractState>;
  using BuggedVars = std::set<Variable*>;
  using SeenContext = std::set<Context>;
  using ContextList = std::vector<Context>;

  void addNotCommittedBug(Variable* var, Variable* pairVar, InstrInfo* ii,
                          InstrInfo* prevII) {
    buggedVars.insert(var);
    buggedVars.insert(pairVar);

    auto varName = var->getName();
    auto prevName = pairVar->getName();
    auto srcLoc = ii->getSrcLoc();
    auto prevLoc = prevII->getSrcLoc();
    NotCommittedBug::report(varName, prevName, srcLoc, prevLoc);
    bugNo++;
  }

  void addSentinelFirstBug(Variable* var, Variable* pairVar, InstrInfo* ii) {
    buggedVars.insert(var);
    buggedVars.insert(pairVar);

    auto varName = var->getName();
    auto prevName = pairVar->getName();
    auto srcLoc = ii->getSrcLoc();

    SentinelFirstBug::report(varName, prevName, srcLoc);
    bugNo++;
  }

  bool checkWriteBug(Variable* var, InstrInfo* ii, AbstractState& state) {
    if (buggedVars.count(var))
      return false;

    for (auto* pair : var->getPairs()) {
      auto* pairVar = pair->getOther(var);

      if (buggedVars.count(pairVar))
        continue;

      auto& pairVal = state[pairVar];

      bool isNotCommitted = (pair->isDcl())
                                ? pairVal.isDclCommitWriteFlush()
                                : isNotCommitted = pairVal.isSclCommitWrite();
      bool isSentinelFirst = pairVal.isUnseen() && pair->isSentinel(var);

      Backtrace backtrace;
      auto* instr = ii->getInstruction();
      auto* prevInstr =
          backtrace.getValueInstruction(instr, pairVar, allResults, contextList);
      auto* prevII = globals.locals.getInstrInfo(prevInstr);

      if (isSentinelFirst) {
        addSentinelFirstBug(var, pairVar, ii);
        return true;
      }

      if (isNotCommitted) {
        addNotCommittedBug(var, pairVar, ii, prevII);
        return true;
      }
    }

    return false;
  }

  void checkWrite(InstrInfo* ii, AbstractState& state) {
    auto* var = ii->getVariable();
    assert(var);

    bool bugFound = checkWriteBug(var, ii, state);
    if (bugFound) {
      return;
    }

    for (auto* wvar : var->getWriteSet()) {
      bugFound = checkWriteBug(wvar, ii, state);
      if (bugFound) {
        return;
      }
    }
  }

  bool addDoubleFlushBug(Variable* var, InstrInfo* ii, AbstractState& state) {
    if (buggedVars.count(var))
      return false;

    auto& val = state[var];

    Backtrace backtrace;
    auto* instr = ii->getInstruction();
    auto* prevInstr =
        backtrace.getValueInstruction(instr, var, allResults, contextList);
    if(!prevInstr)
      return false;

    auto* prevII = globals.locals.getInstrInfo(prevInstr);

    if (val.isDclFlushFlush() && prevII->isFlushBasedInstr()) {
      buggedVars.insert(var);

      auto str = std::string("");

      auto varName = ii->getVarName();
      auto srcLoc = ii->getSrcLoc();

      DoubleFlushBug::report(varName, str, srcLoc, str);
      bugNo++;
      return true;
    }

    return false;
  }

  void checkFlush(InstrInfo* ii, AbstractState& state) {
    auto* var = ii->getVariable();
    assert(var);

    bool bugFound = addDoubleFlushBug(var, ii, state);
    if (bugFound) {
      return;
    }

    for (auto* field : var->getFlushSet()) {
      bugFound = addDoubleFlushBug(field, ii, state);
      if (bugFound) {
        return;
      }
    }
  }

  void checkFinalBugs(AbstractState& state) {
    for (auto& [var, val] : state) {
      if (buggedVars.count(var))
        continue;

      if (globals.locals.inSentinels(var) && !val.isDclFence()) {
        auto mangledName = topFunction->getName();
        auto fncName = globals.dbgInfo.getFunctionName(mangledName).str();
        auto varName = var->getName();
        SentinelCommitBug::report(varName, fncName);
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
      contextList.push_back(newContext);
      checkBugs(f, newContext);
      contextList.pop_back();
      break;
    }
    default:
      return;
    }
  }

  void checkBugs(Function* f, Context& context) {
    if (seen.count(context))
      return;
    seen.insert(context);

    // get function results
    assert(allResults.inAllResults(context));
    auto& results = allResults.getFunctionResults(context);

    // traverse the dataflow codebase
    for (auto& BB : Traversal::getBlocks(f)) {
      for (auto& I : Traversal::getInstructions(&BB)) {
        checkBugs(&I, context, results);
      }
    }
  }

  void resetStructures() {
    topFunction = nullptr;
    buggedVars.clear();
    seen.clear();
    contextList.clear();
    bugNo = 0;
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

  ~BugReporter() { resetStructures(); }

  void checkBugs(Function* f) {
    resetStructures();
    topFunction = f;
    auto context = Context();

    auto mangledName = f->getName();
    auto fncName = globals.dbgInfo.getFunctionName(mangledName);

    checkBugs(f, context);
    auto& finalState = allResults.getFinalState();
    checkFinalBugs(finalState);
    errs() << "Number of bugs in " << fncName << ": " << bugNo << "\n\n\n";
  }
};

} // namespace llvm