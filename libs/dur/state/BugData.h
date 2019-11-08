#pragma once
#include "Common.h"

#include "analysis_util/BugUtil.h"

namespace llvm {

struct CommitPtrBug : public BugData {
  std::string varName;
  std::string curLoc;

  CommitPtrBug(std::string& varName_, std::string& curLoc_)
      : varName(varName_), curLoc(curLoc_) {}

  void print(raw_ostream& O) const {
    errs() << "A) Commit assigned value to " + varName;
    errs() << " at " + curLoc;
    O << "\n";
  }
};

struct DoubleFlushBug : public BugData {
  std::string varName;
  std::string curLoc;
  std::string prevLoc;

  DoubleFlushBug(std::string varName_, std::string curLoc_,
                 std::string prevLoc_)
      : varName(varName_), curLoc(curLoc_), prevLoc(prevLoc_) {}

  void print(raw_ostream& O) const {
    errs() << "B) Double flush " + varName;
    errs() << " at " + curLoc + "\n";
    errs() << "\tFlushed before at " + prevLoc;
    O << "\n";
  }
};

} // namespace llvm
