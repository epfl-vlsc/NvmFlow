#pragma once
#include "Common.h"
#include "FlowTypes.h"
#include "ds/Units.h"

namespace llvm {

class BugReporter {
  struct BugData {
    enum BugType { NotCommittedBug };
    BugType bugType;
    Variable* var;
    Variable* loadedVar;
    Instruction* instr;

    static BugData getNotCommitted(Variable* var_, Variable* loadedVar_,
                                   Instruction* instr_) {
      return {NotCommittedBug, var_, loadedVar_, instr_};
    }

    auto notCommittedBugStr() const {
      std::string name;
      name.reserve(200);

      name += "Commit " + loadedVar->getName() + " for " + var->getName();
      name += " at " + DbgInstr::getSourceLocation(instr);
      name += "\n";

      return name;
    }

    auto getName() const {
      if (bugType == NotCommittedBug)
        return notCommittedBugStr();
    }
  };

  void deleteStructures() {
    if (bugDataList) {
      delete bugDataList;
    }
    if (buggedVars) {
      delete buggedVars;
    }
  }

  void allocStructures() {
    bugDataList = new BugDataList();
    buggedVars = new BuggedVars();
  }

  using BugDataList = std::vector<BugData>;
  using BuggedVars = std::set<Variable*>;

  // data structures
  Units& units;
  Function* currentFunction;
  BugDataList* bugDataList;
  BuggedVars* buggedVars;

public:
  BugReporter(Units& units_) : units(units_) {
    currentFunction = nullptr;
    bugDataList = nullptr;
    buggedVars = nullptr;
  }

  ~BugReporter() {}

  void initUnit(Function* function) {
    currentFunction = function;
    deleteStructures();
    allocStructures();
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

    if (units.variables.inAnnots(var)) {
      auto* loadedVar = ii->getLoadedVariable();
      if (buggedVars->count(loadedVar))
        return;

      // todo find var
      auto& loadedVal = state[loadedVar];
      if (!loadedVal.isFence()) {
        buggedVars->insert(var);
        buggedVars->insert(loadedVar);

        auto* instr = ii->getInstruction();
        auto bugData = BugData::getNotCommitted(var, loadedVar, instr);
        bugDataList->push_back(bugData);
      }
    }
  }

  void checkNotCommittedBug(InstructionInfo* ii, AbstractState& state) {
    auto* var = ii->getVariable();
    assert(var);

    addCheckNotCommittedBug(var, ii, state);
  }
};

} // namespace llvm