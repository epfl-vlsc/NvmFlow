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
    errs() << "*) Log " + varName;
    errs() << " at " + srcLoc + "\n";
  }
};

struct DoubleLogBug : public BugData {
  std::string varName;
  std::string srcLoc;
  std::string prevLoc;

  DoubleLogBug(std::string varName_, std::string srcLoc_, std::string prevLoc_)
      : varName(varName_), srcLoc(srcLoc_), prevLoc(prevLoc_) {}

  void print(raw_ostream& O) const {
    errs() << "*) Double log " + varName;
    errs() << " at " + srcLoc + "\n";
    errs() << "\tLogged before at " + prevLoc + "\n";
  }
};

struct OutTxBug : public BugData {
  std::string varName;
  std::string srcLoc;

  OutTxBug(std::string varName_, std::string srcLoc_)
      : varName(varName_), srcLoc(srcLoc_) {}

  void print(raw_ostream& O) const {
    errs() << "*) Access " + varName;
    errs() << " outside tx at " + srcLoc + "\n";
  }
};

} // namespace llvm
