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

  bool reportWriteBug(Variable* var, InstrInfo* ii, AbstractState& state) {
    if (buggedVars.count(var))
      return false;

    // look at each pair
    for (auto* pair : var->getPairs()) {

      auto* pairVar = pair->getOther(var);

      if (buggedVars.count(pairVar))
        continue;

      auto& pairVal = state[pairVar];

      bool isSentinelFirst = pairVal.isUnseen() && pair->isSentinel(var);
      bool isNotCommitted = (pair->isDcl())
                                ? pairVal.isDclCommitWriteFlush()
                                : isNotCommitted = pairVal.isSclCommitWrite();
      auto eqCommitFn = (pair->isDcl()) ? sameDclCommit : sameSclCommit;

      if (!isSentinelFirst && !isNotCommitted)
        return false;

      // ensure same bug is not reported
      buggedVars.insert(var);
      buggedVars.insert(pairVar);
      bugNo++;

      // bug details
      auto varName = var->getName();
      auto pairVarName = pairVar->getName();
      auto srcLoc = ii->getSrcLoc();

      // check sentinel bug
      if (isSentinelFirst) {
        SentinelFirstBug::report(varName, pairVarName, srcLoc);
        return true;
      }

      if (isNotCommitted) {
        Backtrace backtrace(topFunction);
        auto* instr = ii->getInstruction();
        auto* prevInstr = backtrace.getValueInstruction(
            instr, pairVar, allResults, contextList, eqCommitFn);
        assert(prevInstr);
        auto* prevII = globals.locals.getInstrInfo(prevInstr);
        auto prevLoc = prevII->getSrcLoc();
        NotCommittedBug::report(varName, pairVarName, srcLoc, prevLoc);
        return true;
      }
    }

    return false;
  }

  void checkWrite(InstrInfo* ii, AbstractState& state) {
    auto* var = ii->getVariable();

    if (reportWriteBug(var, ii, state))
      return;

    for (auto* wvar : var->getWriteSet()) {
      if (reportWriteBug(wvar, ii, state))
        return;
    }
  }

  bool reportDoubleFlushBug(Variable* var, InstrInfo* ii,
                            AbstractState& state) {
    if (buggedVars.count(var))
      return false;

    auto& val = state[var];

    Backtrace backtrace(topFunction);
    auto* instr = ii->getInstruction();
    auto* prevInstr = backtrace.getValueInstruction(instr, var, allResults,
                                                    contextList, sameDclFlush);
    if (prevInstr == instr)
      return false;

    auto* prevII = globals.locals.getInstrInfo(prevInstr);
    auto srcLoc = ii->getSrcLoc();
    auto prevLoc = prevII->getSrcLoc();
    bool isDoubleFlush = val.isDclFlushFlush() && prevII->isFlushBasedInstr();
    if (isDoubleFlush) {
      buggedVars.insert(var);

      auto varName = ii->getVarName();
      auto srcLoc = ii->getSrcLoc();
      auto prevLoc = prevII->getSrcLoc();

      DoubleFlushBug::report(varName, varName, srcLoc, prevLoc);
      bugNo++;
      return true;
    }

    return false;
  }

  void checkFlush(InstrInfo* ii, AbstractState& state) {
    auto* var = ii->getVariable();

    if (reportDoubleFlushBug(var, ii, state))
      return;

    for (auto* field : var->getFlushSet()) {
      if (reportDoubleFlushBug(field, ii, state))
        return;
    }
  }

  void checkFinalBugs() {
    auto& state = allResults.getFinalState();
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