#pragma once
#include "Common.h"
#include "FlowTypes.h"
#include "ds/Units.h"

namespace llvm {

class BugReporter {
  using LatVar = FlowTypes::LatVar;
  using LatVal = FlowTypes::LatVal;
  using AbstractState = FlowTypes::AbstractState;

  struct BugData {
    enum BugType { DoubleLog, OutsideTx, NotLogged };
    Instruction* bugStmt;
    Instruction* logStmt;
    LatVar currentVar;
    BugType bugType;

    BugData(Instruction* bugStmt_, Instruction* logStmt_, LatVar currentVar_,
            BugType bugType_)
        : bugStmt(bugStmt_), logStmt(logStmt_), currentVar(currentVar_),
          bugType(bugType_) {}

    static BugData getDoubleLogBug(Instruction* bugStmt_, Instruction* logStmt_,
                                   LatVar currentVar_) {
      return BugData(bugStmt_, logStmt_, currentVar_, DoubleLog);
    }

    static BugData getOutsideTx(Instruction* bugStmt_, LatVar currentVar_) {
      return BugData(bugStmt_, nullptr, currentVar_, OutsideTx);
    }

    static BugData getNotLogged(Instruction* bugStmt_, LatVar currentVar_) {
      return BugData(bugStmt_, nullptr, currentVar_, NotLogged);
    }

    bool isDoubleLog() const { return bugType == DoubleLog; }

    bool isOutsideTx() const { return bugType == OutsideTx; }

    bool isNotLogged() const { return bugType == NotLogged; }

    std::string getMsg() const {
      SmallString<100> buf;
      llvm::raw_svector_ostream os(buf);

      switch (bugType) {
      case DoubleLog:
        os << "Double log " << currentVar->getName() << "\n";
        break;
      case OutsideTx:
        os << "Access to " << currentVar->getName() << " outside transaction\n";
        break;
      case NotLogged:
        os << currentVar->getName() << " is not logged\n";
        break;
      default: {
        report_fatal_error("");
        break;
      }
      }

      return os.str().str();
    }
  };

  using BugDataList = std::vector<BugData>;
  using LastLocationMap = std::map<LatVar, Instruction*>;
  using BuggedVars = std::set<LatVar>;

  // data structures
  Units& units;

  BugDataList* bugDataList;
  LastLocationMap* lastLocationMap;
  BuggedVars* buggedVars;

public:
  BugReporter(Units& units_) : units(units_) {
    bugDataList = new BugDataList();
    lastLocationMap = nullptr;
    buggedVars = nullptr;
  }

  void initUnit() {
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

  void reportBugs() const {
    for (auto& bugData : *bugDataList) {
      // todo report bugs
      errs() << "bug";
    }
  }

  void checkOutsideTxBug(LatVar currentVar, Instruction* bugStmt,
                         AbstractState& state) {
    auto& lv = state[nullptr];

    if (!lv.isInTx()) {
      buggedVars->insert(currentVar);
      auto bugData = BugData::getOutsideTx(bugStmt, currentVar);
      bugDataList->push_back(bugData);
    }
  }

  void checkDoubleLogBug(LatVar currentVar, Instruction* bugStmt,
                         AbstractState& state) {
    if (buggedVars->count(currentVar)) {
      return;
    }

    checkOutsideTxBug(currentVar, bugStmt, state);

    auto& lv = state[currentVar];
    if (lv.isLogged()) {
      buggedVars->insert(currentVar);
      Instruction* logStmt = (*lastLocationMap)[currentVar];
      auto bugData = BugData::getDoubleLogBug(bugStmt, logStmt, currentVar);
      bugDataList->push_back(bugData);
    }
  }

  void checkNotLoggedBug(LatVar currentVar, Instruction* bugStmt,
                         AbstractState& state) {
    if (buggedVars->count(currentVar)) {
      return;
    }

    checkOutsideTxBug(currentVar, bugStmt, state);

    auto& lv = state[currentVar];
    if (!lv.isLogged()) {
      buggedVars->insert(currentVar);
      auto bugData = BugData::getNotLogged(bugStmt, currentVar);
      bugDataList->push_back(bugData);
    }
  }
};

} // namespace llvm