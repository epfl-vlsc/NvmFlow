#pragma once
#include "Common.h"
#include "FlowTypes.h"
#include "ds/Units.h"

namespace llvm {

class BugReporter {
  struct BugData {
    enum BugType { DoubleLog, OutsideTx, NotLogged };
    Instruction* bugInstr;
    Instruction* logInstr;
    LatVar currentVar;
    BugType bugType;

    static BugData getDoubleLogBug(Instruction* bugInstr_,
                                   Instruction* logInstr_, LatVar currentVar_) {
      return {bugInstr_, logInstr_, currentVar_, DoubleLog};
    }

    static BugData getOutsideTx(Instruction* bugInstr_, LatVar currentVar_) {
      return {bugInstr_, nullptr, currentVar_, OutsideTx};
    }

    static BugData getNotLogged(Instruction* bugInstr_, LatVar currentVar_) {
      return {bugInstr_, nullptr, currentVar_, NotLogged};
    }

    bool isDoubleLog() const { return bugType == DoubleLog; }

    bool isOutsideTx() const { return bugType == OutsideTx; }

    bool isNotLogged() const { return bugType == NotLogged; }

    auto getMsg() const {
      std::string msg;
      msg.reserve(200);

      switch (bugType) {
      case DoubleLog:
        msg += "Double log " + currentVar->getName();
        break;
      case OutsideTx:
        msg += "Access to " + currentVar->getName() + " outside transaction";
        break;
      case NotLogged:
        msg += currentVar->getName() += " is not logged";
        break;
      default: {
        report_fatal_error("");
        break;
      }
      }

      msg += " at " + getSourceLocation(bugInstr);
      msg += "\n";

      return msg;
    }
  };

  using BugDataList = std::vector<BugData>;
  using LastLocationMap = std::map<LatVar, Instruction*>;
  using BuggedVars = std::set<LatVar>;

  // data structures
  BugDataList* bugDataList;
  LastLocationMap* lastLocationMap;
  BuggedVars* buggedVars;

public:
  BugReporter() {
    bugDataList = nullptr;
    lastLocationMap = nullptr;
    buggedVars = nullptr;
  }

  void initUnit() {
    if (bugDataList) {
      delete bugDataList;
    }
    bugDataList = new BugDataList();

    if (lastLocationMap) {
      delete lastLocationMap;
    }
    lastLocationMap = new LastLocationMap();

    if (buggedVars) {
      delete buggedVars;
    }
    buggedVars = new BuggedVars();
  }

  void updateLastLocation(LatVar var, Instruction* i) {
    (*lastLocationMap)[var] = i;
  }

  void print(raw_ostream& O) const {
    O << "bugs\n";
    for (auto& bugData : *bugDataList) {
      errs() << bugData.getMsg();
    }
    O << "---------------------------------\n";
  }

  void checkOutsideTxBug(Instruction* bugInstr, LatVar currentVar, LatVar txVar,
                         AbstractState& state) {
    auto& lv = state[txVar];

    if (!lv.inTx()) {
      buggedVars->insert(currentVar);
      auto bugData = BugData::getOutsideTx(bugInstr, currentVar);
      bugDataList->push_back(bugData);
    }
  }

  void checkDoubleLogBug(Instruction* bugInstr, LatVar currentVar, LatVar txVar,
                         AbstractState& state) {
    if (buggedVars->count(currentVar)) {
      return;
    }

    checkOutsideTxBug(bugInstr, currentVar, txVar, state);

    auto& lv = state[currentVar];
    if (lv.isLogged()) {
      buggedVars->insert(currentVar);
      Instruction* logInstr = (*lastLocationMap)[currentVar];
      auto bugData = BugData::getDoubleLogBug(bugInstr, logInstr, currentVar);
      bugDataList->push_back(bugData);
    }
  }

  void checkNotLoggedBug(Instruction* bugInstr, LatVar currentVar, LatVar txVar,
                         AbstractState& state) {
    if (buggedVars->count(currentVar)) {
      return;
    }

    checkOutsideTxBug(bugInstr, currentVar, txVar, state);

    auto& lv = state[currentVar];
    if (!lv.isLogged()) {
      buggedVars->insert(currentVar);
      auto bugData = BugData::getNotLogged(bugInstr, currentVar);
      bugDataList->push_back(bugData);
    }
  }
};

} // namespace llvm