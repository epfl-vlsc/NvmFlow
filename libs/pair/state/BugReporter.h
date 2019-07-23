#pragma once
#include "Common.h"
#include "FlowTypes.h"
#include "ds/Units.h"

namespace llvm {

class BugReporter {
  struct BugData {
    Instruction* bugInstr;
    Variable* bugVar;
    PairVariable* pairVar;

    static BugData getNotCommitted(Instruction* bugInstr_, Variable* bugVar_,
                                   PairVariable* pairVar_) {
      return {bugInstr_, bugVar_, pairVar_};
    }

    auto getName() const {
      std::string name;
      name.reserve(200);

      auto* pair = pairVar->getPair(bugVar);

      name += "Not committed " + pair->getName() + " for " + bugVar->getName();

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
  Function* currentFunction;
  BugDataList* bugDataList;
  // LastLocationMap* lastLocationMap;
  BuggedVars* buggedVars;

public:
  BugReporter() {
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
    /*
    auto& lv = state[currentVar];

    buggedVars->insert(currentVar);
    auto bugData = BugData::getOutsideTx(bugInstr, currentVar);
    bugDataList->push_back(bugData);
     */
  }
};

} // namespace llvm