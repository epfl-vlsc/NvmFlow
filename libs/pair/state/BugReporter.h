#pragma once
#include "BugData.h"
#include "Common.h"
#include "FlowTypes.h"
#include "analysis_util/DataflowResults.h"
#include "analysis_util/DfUtil.h"
#include "ds/Globals.h"

namespace llvm {

class BugReporter {
  using AllResults = DataflowResults<AbstractState>;
  using BugDataList = std::vector<BugData*>;
  using LastLocationMap = std::map<Variable*, Instruction*>;
  using BuggedVars = std::set<Variable*>;
  using SeenContext = std::set<Context>;

  void resetStructures() {
    for (auto* bd : bugDataList) {
      delete bd;
    }
    bugDataList.clear();
    lastLocationMap.clear();
    buggedVars.clear();
  }

  void updateLastLocation(Variable* var, InstrInfo* ii) {
    auto* instr = ii->getInstruction();
    assert(instr);
    lastLocationMap[var] = instr;
  }

  auto* getLastLocation(Variable* var) {
    assertInDs(lastLocationMap, var);
    return lastLocationMap[var];
  }

  void print(raw_ostream& O) const {
    auto mangledName = topFunction->getName();
    auto fncName = globals.dbgInfo.getFunctionName(mangledName);
    O << fncName << " bugs\n";
    for (auto& bugData : bugDataList) {
      errs() << bugData->getName();
    }
    O << "---------------------------------\n";
    O << "\n\n\n";
  }

  void addNotCommittedBug(Variable* var, Variable* pairVar, InstrInfo* ii) {
    buggedVars.insert(var);
    buggedVars.insert(pairVar);

    auto* pairInst = getLastLocation(pairVar);
    auto* previi = globals.locals.getInstrInfo(pairInst);

    auto varName = ii->getVarName();
    auto prevName = previi->getVarName();
    auto srcLoc = ii->getSrcLoc();
    auto prevLoc = previi->getSrcLoc();

    auto* bugData =
        BugFactory::getNotCommittedBug(varName, prevName, srcLoc, prevLoc);
    bugDataList.push_back(bugData);
  }

  void addSentinelFirstBug(Variable* var, Variable* pairVar, InstrInfo* ii,
                           AbstractState& state) {
    buggedVars.insert(var);
    buggedVars.insert(pairVar);

    auto varName = ii->getVarName();
    auto prevName = pairVar->getName();
    auto srcLoc = ii->getSrcLoc();

    auto* bugData = BugFactory::getSentinelFirstBug(varName, prevName, srcLoc);
    bugDataList.push_back(bugData);
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

      if (isSentinelFirst) {
        addSentinelFirstBug(var, pairVar, ii);
        return true;
      }

      if (isNotCommitted) {
        addNotCommittedBug(var, pairVar, ii);
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

    if (val.isDclFlushFlush()) {
      buggedVars.insert(var);

      auto* prevInstr = getLastLocation(var);
      auto* previi = globals.locals.getInstrInfo(prevInstr);

      auto varName = ii->getVarName();
      auto prevName = previi->getVarName();
      auto srcLoc = ii->getSrcLoc();
      auto prevLoc = previi->getSrcLoc();

      auto* bugData =
          BugFactory::getDoubleFlushBug(varName, prevName, srcLoc, prevLoc);
      bugDataList.push_back(bugData);
      return true;
    }

    return false;
  }

  void checkDoubleFlushBug(InstrInfo* ii, AbstractState& state) {
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
      if (globals.locals.inSentinels(var) && !val.isDclFence()) {
        auto mangledName = topFunction->getName();
        auto fncName = globals.dbgInfo.getFunctionName(mangledName).str();
        auto varName = var->getName();
        auto* bugData = BugFactory::getSentinelCommitBug(varName, fncName);
        bugDataList.push_back(bugData);
      }
    }
  }

  template <typename FunctionResults>
  void checkPairBugs(Instruction* i, Context& context,
                     FunctionResults& results) {
    auto* ii = globals.locals.getInstrInfo(i);
    if (!ii)
      return;

    auto* instKey = Traversal::getInstructionKey(i);
    auto& state = results[instKey];

    switch (ii->getInstrType()) {
    case InstrInfo::WriteInstr:
      checkWrite(ii, state);
      break;
    case InstrInfo::FlushInstr:
      // checkFlush(ii, state, false);
      break;
    case InstrInfo::FlushFenceInstr:
      // checkFlush(ii, state, true);
      break;
    case InstrInfo::IpInstr: {
      auto* ci = dyn_cast<CallInst>(i);
      auto* f = ci->getCalledFunction();
      Context newContext(context, ci);
      checkPairBugs(f, newContext);
      break;
    }
    default:
      return;
    }
  }

  void checkPairBugs(Function* f, Context& context) {
    if (seen.count(context))
      return;
    seen.insert(context);

    for (auto& BB : Traversal::getBlocks(f)) {
      for (auto& I : Traversal::getInstructions(&BB)) {
        auto& results = allResults.getFunctionResults(context);
        checkPairBugs(&I, context, results);
      }
    }
  }

  // data structures
  Globals& globals;
  Function* topFunction;
  AllResults& allResults;
  BugDataList bugDataList;
  LastLocationMap lastLocationMap;
  BuggedVars buggedVars;
  SeenContext seen;

public:
  BugReporter(Globals& globals_, AllResults& allResults_)
      : globals(globals_), topFunction(nullptr), allResults(allResults_) {}

  ~BugReporter() { resetStructures(); }

  void checkBugs(Function* f) {
    resetStructures();
    topFunction = f;
    auto context = Context();
    checkPairBugs(f, context);
    auto& finalState = allResults.getFinalState(f);
    checkFinalBugs(finalState);
  }
};

} // namespace llvm