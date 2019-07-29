#pragma once
#include "Common.h"
#include "FlowTypes.h"
#include "ds/Units.h"

namespace llvm {

class BugReporter {
  struct BugData {
    enum BugType { NotCommittedBug, DoubleFlushBug };
    BugType bugType;
    Variable* var;
    Instruction* instr;
    Instruction* prevInstr;
    Variable* prevVar;

    static BugData getNotCommitted(Variable* var_, Instruction* instr_,
                                   Variable* prevVar_) {
      assert(prevVar_);
      return {NotCommittedBug, var_, instr_, nullptr, prevVar_};
    }

    static BugData getDoubleFlush(Variable* var_, Instruction* instr_,
                                  Instruction* prevInstr_) {
      assert(prevInstr_);
      return {DoubleFlushBug, var_, instr_, prevInstr_, nullptr};
    }

    auto notCommittedBugStr() const {
      std::string name;
      name.reserve(200);

      name += "Commit " + prevVar->getName() + " for " + var->getName();
      name += " at " + DbgInstr::getSourceLocation(instr);
      name += "\n";

      return name;
    }

    auto doubleFlushBugStr() const {
      std::string name;
      name.reserve(200);

      name += "Double flush " + var->getName();
      name += " at " + DbgInstr::getSourceLocation(instr) + "\n";
      name += " prev at " + DbgInstr::getSourceLocation(prevInstr);
      name += "\n";

      return name;
    }

    auto getName() const {
      if (bugType == NotCommittedBug)
        return notCommittedBugStr();
      else
        return doubleFlushBugStr();
    }
  };

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

  using BugDataList = std::vector<BugData>;
  using LastLocationMap = std::map<Variable*, Instruction*>;
  using BuggedVars = std::set<Variable*>;

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
    assert(lastLocationMap->count(var));
    return (*lastLocationMap)[var];
  }

  void print(raw_ostream& O) const {
    O << currentFunction->getName() << " bugs\n";
    for (auto& bugData : *bugDataList) {
      errs() << bugData.getName();
    }
    O << "---------------------------------\n";
  }

  void addCheckNotCommittedBug(Variable* var, InstructionInfo* ii,
                               AbstractState& state) {
    if (buggedVars->count(var))
      return;

    for (auto* pair : units.variables.getPairs(var)) {
      auto* pairVar = pair->getPair(var);

      if (buggedVars->count(pairVar))
        continue;

      auto& pairVal = state[pairVar];
      bool badPairValStates = (pair->isDcl())
                                  ? pairVal.isWriteDcl() || pairVal.isFlushDcl()
                                  : pairVal.isWriteScl();

      if (badPairValStates) {
        buggedVars->insert(var);
        buggedVars->insert(pairVar);

        auto* instr = ii->getInstruction();
        auto bugData = BugData::getNotCommitted(var, instr, pairVar);
        bugDataList->push_back(bugData);
      }
    }
  }

  void checkNotCommittedBug(InstructionInfo* ii, AbstractState& state) {
    auto* var = ii->getVariable();
    assert(var);

    addCheckNotCommittedBug(var, ii, state);

    if (var->isObj()) {
      for (auto* obj : units.variables.getWriteObjs(var)) {
        addCheckNotCommittedBug(obj, ii, state);
      }
    }
  }

  void addDoubleFlushBug(Variable* var, InstructionInfo* ii,
                         AbstractState& state) {
    if (buggedVars->count(var))
      return;

    auto& val = state[var];

    if (val.isFenceDcl() || val.isFlushDcl()) {
      auto* instr = ii->getInstruction();
      auto* prevInstr = getLastLocation(var);
      auto bugData = BugData::getDoubleFlush(var, instr, prevInstr);
      bugDataList->push_back(bugData);
    }
  }

  void checkDoubleFlushBug(InstructionInfo* ii, AbstractState& state) {
    auto* var = ii->getVariable();
    assert(var);

    addDoubleFlushBug(var, ii, state);

    if (var->isObj()) {
      for (auto* field : units.variables.getFlushFields(var)) {
        addDoubleFlushBug(field, ii, state);
      }
    }
  }
};

} // namespace llvm