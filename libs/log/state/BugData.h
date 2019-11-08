#pragma once
#include "Common.h"

#include "analysis_util/BugUtil.h"

namespace llvm {

struct CommitBug : public BugData {
  std::string varName;
  std::string srcLoc;

  CommitBug(std::string& varName_, std::string& srcLoc_)
      : varName(varName_), srcLoc(srcLoc_) {}

  void print(raw_ostream& O) const {
    O << "A) Log " + varName;
    O << " at " + srcLoc;
    O << "\n";
  }
};

struct DoubleLogBug : public BugData {
  std::string varName;
  std::string srcLoc;
  std::string prevLoc;

  DoubleLogBug(std::string varName_, std::string srcLoc_, std::string prevLoc_)
      : varName(varName_), srcLoc(srcLoc_), prevLoc(prevLoc_){}

  void print(raw_ostream& O) const {
    O << "B) Double log " + varName;
    O << " at " + srcLoc + "\n";
    O << "\tLogged before at " + prevLoc;
    O << "\n";
  }
};

struct OutTxBug : public BugData {
  std::string varName;
  std::string srcLoc;

  OutTxBug(std::string varName_, std::string srcLoc_)
      : varName(varName_), srcLoc(srcLoc_) {}

  void print(raw_ostream& O) const {
    O << "C) Access " + varName;
    O << " outside tx at " + srcLoc;
    O << "\n";
  }
};

} // namespace llvm
