#pragma once
#include "Common.h"
#include "FlowTypes.h"
#include "ds/Units.h"

namespace llvm {

class BugReporter {
  struct BugData {
    enum BugType { DoubleLogBug, NotCommittedBug, OutsideTxBug };
    BugType bugType;
    InstructionInfo* ii;
    InstructionInfo* previi;

    static BugData getDoubleLog(InstructionInfo* ii_,
                                InstructionInfo* previi_) {
      assert(ii_ && previi_);
      return {DoubleLogBug, ii_, previi_};
    }

    static BugData getNotCommitted(InstructionInfo* ii_) {
      assert(ii_);
      return {NotCommittedBug, ii_, nullptr};
    }

    static BugData getOutsideTx(InstructionInfo* ii_) {
      assert(ii_);
      return {OutsideTxBug, ii_, nullptr};
    }

    bool isDoubleLog() const { return bugType == DoubleLogBug; }

    bool isNotCommitted() const { return bugType == NotCommittedBug; }

    bool isOutsideTx() const { return bugType == OutsideTxBug; }

    auto doubleLogBugStr() const {
      std::string name;
      name.reserve(200);

      name += "Double log " + ii->getVariableName();
      name += " at " + ii->getInstructionName() + "\n";
      name += "\tLogged before " + previi->getVariableName();
      name += " at " + previi->getInstructionName() + "\n";

      return name;
    }

    auto notCommittedBugStr() const {
      std::string name;
      name.reserve(200);

      name += "Commit " + ii->getVariableName();
      name += " at " + ii->getInstructionName() + "\n";

      return name;
    }

    auto outsideTxBugStr() const {
      std::string name;
      name.reserve(200);

      name += "Access to " + ii->getVariableName();
      name += " outside tx at " + ii->getInstructionName() + "\n";

      return name;
    }

    auto getName() const {
      if (bugType == DoubleLogBug)
        return doubleLogBugStr();
      else if (bugType == NotCommittedBug)
        return notCommittedBugStr();
      else
        return outsideTxBugStr();
    }
  };

  using BugDataList = std::vector<BugData>;
  using LastLocationMap = std::map<LatVar, Instruction*>;
  using BuggedVars = std::set<LatVar>;

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
    O << currentFunction->getName() << " bugs\n";
    for (auto& bugData : *bugDataList) {
      errs() << bugData.getName();
    }
    O << "---------------------------------\n";
  }

  void addOutsideTxBug(Variable* txVar, InstructionInfo* ii,
                       AbstractState& state) {
    auto* var = ii->getVariable();
    assert(var);

    if (buggedVars->count(txVar) || buggedVars->count(var))
      return;

    auto& lv = state[txVar];

    if (!lv.inTx()) {
      buggedVars->insert(var);

      auto bugData = BugData::getOutsideTx(ii);
      bugDataList->push_back(bugData);
    }
  }

  bool addDoubleLogBug(Variable* var, InstructionInfo* ii,
                       AbstractState& state) {
    if (buggedVars->count(var))
      return false;

    auto& val = state[var];

    if (val.isLogged()) {
      buggedVars->insert(var);

      auto* prevInstr = getLastLocation(var);
      auto* previi = units.variables.getInstructionInfo(prevInstr);

      auto bugData = BugData::getDoubleLog(ii, previi);
      bugDataList->push_back(bugData);
      return true;
    }

    return false;
  }

  void checkDoubleLogBug(Variable* txVar, InstructionInfo* ii,
                         AbstractState& state) {
    addOutsideTxBug(txVar, ii, state);

    auto* var = ii->getVariable();
    assert(var);

    bool bugFound = addDoubleLogBug(var, ii, state);
    if (bugFound) {
      return;
    }

    if (var->isObj()) {
      for (auto* field : units.variables.getFlushFields(var)) {
        bugFound = addDoubleLogBug(field, ii, state);
        if (bugFound) {
          return;
        }
      }
    }
  }

  void addNotCommittedBug(Variable* var, InstructionInfo* ii,
                          AbstractState& state) {
    if (buggedVars->count(var))
      return;

    auto& val = state[var];

    if (!val.isLogged()) {
      buggedVars->insert(var);
      auto bugData = BugData::getNotCommitted(ii);
      bugDataList->push_back(bugData);
    }
  }

  void checkNotCommittedBug(Variable* txVar, InstructionInfo* ii,
                            AbstractState& state) {
    addOutsideTxBug(txVar, ii, state);

    auto* var = ii->getVariable();
    assert(var);

    addNotCommittedBug(var, ii, state);
  }
};

} // namespace llvm