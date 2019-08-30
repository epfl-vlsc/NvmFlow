#pragma once
#include "Common.h"
#include "FlowTypes.h"
#include "ds/Globals.h"

namespace llvm {

class BugReporter {
  struct BugData {
    enum BugType { NotCommittedBug, DoubleFlushBug };
    BugType bugType;
    InstrInfo* ii;
    InstrInfo* previi;

    static BugData getNotCommitted(InstrInfo* ii_, InstrInfo* previi_) {
      assert(ii_ && previi_);
      return {NotCommittedBug, ii_, previi_};
    }

    static BugData getDoubleFlush(InstrInfo* ii_, InstrInfo* previi_) {
      assert(ii_ && previi_);
      return {DoubleFlushBug, ii_, previi_};
    }

    auto notCommittedBugStr() const {
      std::string name;
      name.reserve(200);

      auto* var = ii->getVariable();
      auto* instr = ii->getInstruction();
      auto* prevVar = previi->getVariable();
      auto* prevInstr = previi->getInstruction();

      name += "For " + var->getName();
      name += " at " + DbgInstr::getSourceLocation(instr) + "\n";
      name += "\tCommit " + prevVar->getName();
      name += " at " + DbgInstr::getSourceLocation(prevInstr) + "\n";

      return name;
    }

    auto doubleFlushBugStr() const {
      std::string name;
      name.reserve(200);

      auto* var = ii->getVariable();
      auto* instr = ii->getInstruction();
      auto* prevVar = previi->getVariable();
      auto* prevInstr = previi->getInstruction();

      name += "Double flush " + var->getName();
      name += " at " + DbgInstr::getSourceLocation(instr) + "\n";
      name += "\tFlushed before " + prevVar->getName();
      name += " at " + DbgInstr::getSourceLocation(prevInstr) + "\n";

      return name;
    }

    auto getName() const {
      if (bugType == NotCommittedBug)
        return notCommittedBugStr();
      else
        return doubleFlushBugStr();
    }
  };

  using BugDataList = std::vector<BugData>;
  using LastLocationMap = std::map<Variable*, Instruction*>;
  using BuggedVars = std::set<Variable*>;

  void deleteStructures() {
    if (bugDataList) {
      delete bugDataList;
    }
    if (lastLocationMap) {
      delete lastLocationMap;
    }
    if (buggedVars) {
      delete buggedVars;
    }
  }

  void allocStructures() {
    bugDataList = new BugDataList();
    lastLocationMap = new LastLocationMap();
    buggedVars = new BuggedVars();
  }

  // data structures
  Globals& globals;
  Function* currentFunction;
  BugDataList* bugDataList;
  LastLocationMap* lastLocationMap;
  BuggedVars* buggedVars;

public:
  BugReporter(Globals& globals_) : globals(globals_) {
    currentFunction = nullptr;
    bugDataList = nullptr;
    lastLocationMap = nullptr;
    buggedVars = nullptr;
  }

  ~BugReporter() {}

  void initUnit(Function* function) {
    currentFunction = function;
    deleteStructures();
    allocStructures();
  }

  void updateLastLocation(Variable* var, InstrInfo* ii) {
    auto* instr = ii->getInstruction();
    assert(instr);
    (*lastLocationMap)[var] = instr;
  }

  auto* getLastLocation(Variable* var) {
    assertInDs(lastLocationMap, var);
    return (*lastLocationMap)[var];
  }

  void print(raw_ostream& O) const {
    auto mangledName = currentFunction->getName();
    auto fncName = globals.dbgInfo.getFunctionName(mangledName);
    O << fncName << " bugs\n";
    for (auto& bugData : *bugDataList) {
      errs() << bugData.getName();
    }
    O << "---------------------------------\n";
    O << "\n\n\n";
  }

  bool addCheckNotCommittedBug(Variable* var, InstrInfo* ii,
                               AbstractState& state) {
    if (buggedVars->count(var))
      return false;

    for (auto* pair : var->getPairs()) {
      auto* pairVar = pair->getOther(var);

      if (buggedVars->count(pairVar))
        continue;

      auto& pairVal = state[pairVar];
      bool badPairValStates = (pair->isDcl()) ? pairVal.isDclCommitWrite() ||
                                                    pairVal.isDclCommitFlush()
                                              : pairVal.isSclCommitWrite();
      if (badPairValStates) {
        buggedVars->insert(var);
        buggedVars->insert(pairVar);

        auto* pairInst = getLastLocation(pairVar);
        auto* previi = globals.locals.getInstrInfo(pairInst);

        auto bugData = BugData::getNotCommitted(ii, previi);
        bugDataList->push_back(bugData);
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
    if (buggedVars->count(var))
      return false;

    auto& val = state[var];

    if (val.isDclFlushFlush()) {
      buggedVars->insert(var);

      auto* prevInstr = getLastLocation(var);
      auto* previi = globals.locals.getInstrInfo(prevInstr);

      auto bugData = BugData::getDoubleFlush(ii, previi);
      bugDataList->push_back(bugData);
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
};

} // namespace llvm