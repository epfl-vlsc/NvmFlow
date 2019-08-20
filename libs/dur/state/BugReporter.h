#pragma once
#include "Common.h"
#include "FlowTypes.h"
#include "ds/Units.h"

namespace llvm {

class BugReporter {
  struct BugData {
    enum BugType { NotCommittedBug };
    BugType bugType;
    InstructionInfo* ii;

    static BugData getNotCommitted(InstructionInfo* ii_) {
      assert(ii_);
      return {NotCommittedBug, ii_};
    }

    auto notCommittedBugStr() const {
      std::string name;
      name.reserve(200);

      name += "For " + ii->getVariableName();
      name += " commit " + ii->getVariableName(true);
      name += " at " + ii->getInstructionName() + "\n";

      return name;
    }

    auto getName() const { return notCommittedBugStr(); }
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
    O << "\n\n\n";
  }

  void checkNotCommittedBug(InstructionInfo* ii, AbstractState& state) {
    auto* var = ii->getVariable();
    auto* loadVar = ii->getLoadVariable();

    auto* loadVarAg = loadVar->getAliasGroup();

    if (buggedVars->count(loadVarAg))
      return;

    if (var->isAnnotatedField()) {
      auto& val = state[loadVarAg];

      if (!val.isFence()) {
        auto bugData = BugData::getNotCommitted(ii);
        bugDataList->push_back(bugData);
      }
    }
  }
};

} // namespace llvm