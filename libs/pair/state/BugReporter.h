#pragma once
#include "Common.h"
#include "FlowTypes.h"
#include "ds/Units.h"

namespace llvm {

class BugReporter {
  struct BugData {
    Instruction* bugInstr;
    Variable* bugVar;
    Variable* pairVar;

    static BugData getNotCommitted(Instruction* bugInstr_, Variable* bugVar_,
                                   Variable* pairVar_) {
      return {bugInstr_, bugVar_, pairVar_};
    }

    auto getName() const {
      std::string name;
      name.reserve(200);

      name += "Commit " + pairVar->getName() + " for " + bugVar->getName();
      name += " at " + DbgInstr::getSourceLocation(bugInstr);
      name += "\n";

      return name;
    }
  };

  void deleteStructures() {
    if (bugDataList) {
      delete bugDataList;
    }
    /*
    if (lastLocationMap) {
      delete lastLocationMap;
    }
    */
    if (buggedVars) {
      delete buggedVars;
    }
  }

  void allocStructures() {
    bugDataList = new BugDataList();
    // lastLocationMap = new LastLocationMap();
    buggedVars = new BuggedVars();
  }

  using BugDataList = std::vector<BugData>;
  // using LastLocationMap = std::map<Variable*, Instruction*>;
  using BuggedVars = std::set<Variable*>;

  // data structures
  Units& units;
  Function* currentFunction;
  BugDataList* bugDataList;
  // LastLocationMap* lastLocationMap;
  BuggedVars* buggedVars;

public:
  BugReporter(Units& units_) : units(units_) {
    currentFunction = nullptr;
    bugDataList = nullptr;
    // lastLocationMap = nullptr;
    buggedVars = nullptr;
  }

  ~BugReporter() {}

  void initUnit(Function* function) {
    currentFunction = function;
    deleteStructures();
    allocStructures();
  }
  /*
    void updateLastLocation(Instruction* i, Variable* var) {
      (*lastLocationMap)[var] = i;
    }
   */
  void print(raw_ostream& O) const {
    O << currentFunction->getName() << " bugs\n";
    for (auto& bugData : *bugDataList) {
      errs() << bugData.getName();
    }
    O << "---------------------------------\n";
  }

  void checkNotCommittedBug(InstructionInfo* ii, AbstractState& state) {
    auto* var = ii->getVariable();
    auto* instr = ii->getInstruction();
    assert(var && instr);
    if (buggedVars->count(var))
      return;

    for (auto* pair : units.variables.getPairs(var)) {
      auto* pairVar = pair->getPair(var);
      if (buggedVars->count(pairVar))
        continue;

      auto& pairVal = state[pairVar];
      if (pairVal.isWriteDcl() || pairVal.isWriteScl() || pairVal.isFlushDcl()) {
        buggedVars->insert(var);
        buggedVars->insert(pairVar);

        auto bugData = BugData::getNotCommitted(instr, var, pairVar);
        bugDataList->push_back(bugData);
      }
    }
  }
};

} // namespace llvm