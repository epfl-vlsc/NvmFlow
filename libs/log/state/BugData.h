#pragma once
#include "Common.h"

#include "analysis_util/BugUtil.h"

namespace llvm {

struct CommitBug : public BugData {
  std::string varName;
  std::string curLoc;

  CommitBug(std::string& varName_, std::string& curLoc_)
      : varName(varName_), curLoc(curLoc_) {}

  void print(raw_ostream& O) const {
    O << "A) Log " + varName;
    O << " at " + curLoc;
    O << "\n";
  }
};

struct DoubleLogBug : public BugData {
  std::string varName;
  std::string curLoc;
  std::string prevLoc;

  DoubleLogBug(std::string varName_, std::string curLoc_, std::string prevLoc_)
      : varName(varName_), curLoc(curLoc_), prevLoc(prevLoc_){}

  void print(raw_ostream& O) const {
    O << "B) Double log " + varName;
    O << " at " + curLoc + "\n";
    O << "\tLogged before at " + prevLoc;
    O << "\n";
  }
};

struct OutTxBug : public BugData {
  std::string varName;
  std::string curLoc;

  OutTxBug(std::string varName_, std::string curLoc_)
      : varName(varName_), curLoc(curLoc_) {}

  void print(raw_ostream& O) const {
    O << "C) Access " + varName;
    O << " outside tx at " + curLoc;
    O << "\n";
  }
};

} // namespace llvm
