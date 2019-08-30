#pragma once
#include "BugData.h"
#include "Common.h"
#include "FlowTypes.h"
#include "ds/Globals.h"

namespace llvm {

class BugReporter {
  using BugDataList = std::vector<BugData*>;
  using LastLocationMap = std::map<Variable*, Instruction*>;
  using BuggedVars = std::set<Variable*>;

  void deleteStructures() {
    for (auto* bd : bugDataList) {
      delete bd;
    }
    bugDataList.clear();
    lastLocationMap.clear();
    buggedVars.clear();
  }

  // data structures
  Globals& globals;
  Function* currentFunction;
  BugDataList bugDataList;
  LastLocationMap lastLocationMap;
  BuggedVars buggedVars;

public:
  BugReporter(Globals& globals_) : globals(globals_) {
    currentFunction = nullptr;
  }

  ~BugReporter() {}

  void initUnit(Function* function) {
    currentFunction = function;
    deleteStructures();
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
    auto mangledName = currentFunction->getName();
    auto fncName = globals.dbgInfo.getFunctionName(mangledName);
    O << fncName << " bugs\n";
    for (auto& bugData : bugDataList) {
      errs() << bugData->getName();
    }
    O << "---------------------------------\n";
    O << "\n\n\n";
  }

  bool addCheckNotCommittedBug(Variable* var, InstrInfo* ii,
                               AbstractState& state) {
    if (buggedVars.count(var))
      return false;

    for (auto* pair : var->getPairs()) {
      auto* pairVar = pair->getOther(var);

      if (buggedVars.count(pairVar))
        continue;

      auto& pairVal = state[pairVar];

      bool isNotCommitted = false;
      bool isSentinelFirst = pairVal.isUnseen() && pair->isSentinel(var);

      if (pair->isDcl()) {
        isNotCommitted =
            pairVal.isDclCommitWrite() || pairVal.isDclCommitFlush();
      } else {
        isNotCommitted = pairVal.isSclCommitWrite();
      }

      if (isSentinelFirst) {
        buggedVars.insert(var);
        buggedVars.insert(pairVar);

        auto varName = ii->getVarName();
        auto prevName = pairVar->getName();
        auto srcLoc = ii->getSrcLoc();

        auto* bugData =
            BugFactory::getSentinelFirstBug(varName, prevName, srcLoc);
        bugDataList.push_back(bugData);
        return true;
      }

      if (isNotCommitted) {
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
        return true;
      }
    }

    return false;
  }

  void checkNotCommittedBug(InstrInfo* ii, AbstractState& state) {
    auto* var = ii->getVariable();
    assert(var);

    bool bugFound = addCheckNotCommittedBug(var, ii, state);
    if (bugFound) {
      return;
    }

    for (auto* wvar : var->getWriteSet()) {
      bugFound = addCheckNotCommittedBug(wvar, ii, state);
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
        auto mangledName = currentFunction->getName();
        auto fncName = globals.dbgInfo.getFunctionName(mangledName).str();
        auto varName = var->getName();
        auto* bugData = BugFactory::getSentinelCommitBug(varName, fncName);
        bugDataList.push_back(bugData);
      }
    }
  }

  template<typename DfResults>
  void checkBugs(DfResults& dfResults){
    errs() << "lol\n";
  }
};

} // namespace llvm