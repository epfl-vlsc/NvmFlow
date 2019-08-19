#pragma once
#include "Common.h"
#include "FlowTypes.h"
#include "ds/Units.h"

namespace llvm {

class BugReporter {
  struct BugData {
    enum BugType { NotCommittedBug, DoubleFlushBug };
    BugType bugType;
    InstructionInfo* ii;
    InstructionInfo* previi;

    static BugData getNotCommitted(InstructionInfo* ii_,
                                   InstructionInfo* previi_) {
      assert(ii_ && previi_);
      return {NotCommittedBug, ii_, previi_};
    }

    static BugData getDoubleFlush(InstructionInfo* ii_,
                                  InstructionInfo* previi_) {
      assert(ii_ && previi_);
      return {DoubleFlushBug, ii_, previi_};
    }

    auto notCommittedBugStr() const {
      std::string name;
      name.reserve(200);

      name += "For " + ii->getVariableName();
      name += " at " + ii->getInstructionName() + "\n";
      name += "\tCommit " + previi->getVariableName();
      name += " at " + previi->getInstructionName() + "\n";

      return name;
    }

    auto doubleFlushBugStr() const {
      std::string name;
      name.reserve(200);

      name += "Double flush " + ii->getVariableName();
      name += " at " + ii->getInstructionName() + "\n";
      name += "\tFlushed before " + previi->getVariableName();
      name += " at " + previi->getInstructionName() + "\n";

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
  Units& units;
  Function* currentFunction;
  BugDataList* bugDataList;
  LastLocationMap* lastLocationMap;
  BuggedVars* buggedVars;

public:
  BugReporter(Units& units_) : units(units_) {
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

  void updateLastLocation(Variable* var, InstructionInfo* ii) {
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
    auto fncName = units.dbgInfo.getFunctionName(mangledName);
    O << fncName << " bugs\n";
    for (auto& bugData : *bugDataList) {
      errs() << bugData.getName();
    }
    O << "---------------------------------\n";
  }

  bool addCheckNotCommittedBug(Variable* var, InstructionInfo* ii,
                               AbstractState& state) {
    if (buggedVars->count(var))
      return false;

    for (auto* pair : units.variables.getPairs(var)) {
      auto* pairVar = pair->getPair(var);

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
        auto* previi = units.variables.getInstructionInfo(pairInst);

        auto bugData = BugData::getNotCommitted(ii, previi);
        bugDataList->push_back(bugData);
        return true;
      }
    }

    return false;
  }

  void checkNotCommittedBug(InstructionInfo* ii, AbstractState& state) {
    auto* var = ii->getVariable();
    assert(var);

    bool bugFound = addCheckNotCommittedBug(var, ii, state);
    if (bugFound) {
      return;
    }

    if (var->isField()) {
      for (auto* obj : units.variables.getWriteObjs(var)) {
        bugFound = addCheckNotCommittedBug(obj, ii, state);
        if (bugFound) {
          return;
        }
      }
    }
  }

  bool addDoubleFlushBug(Variable* var, InstructionInfo* ii,
                         AbstractState& state) {
    if (buggedVars->count(var))
      return false;

    auto& val = state[var];

    if (val.isDclFlushFlush()) {
      buggedVars->insert(var);

      auto* prevInstr = getLastLocation(var);
      auto* previi = units.variables.getInstructionInfo(prevInstr);

      auto bugData = BugData::getDoubleFlush(ii, previi);
      bugDataList->push_back(bugData);
      return true;
    }

    return false;
  }

  void checkDoubleFlushBug(InstructionInfo* ii, AbstractState& state) {
    auto* var = ii->getVariable();
    assert(var);

    bool bugFound = addDoubleFlushBug(var, ii, state);
    if (bugFound) {
      return;
    }

    if (var->isObj()) {
      for (auto* field : units.variables.getFlushFields(var)) {
        bugFound = addDoubleFlushBug(field, ii, state);
        if (bugFound) {
          return;
        }
      }
    }
  }
};

} // namespace llvm