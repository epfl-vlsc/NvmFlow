#pragma once
#include "Common.h"

#include "analysis_util/BugUtil.h"

namespace llvm {

struct CommitPtrBug : public BugData {
  std::string varName;
  std::string srcLoc;

  CommitPtrBug(std::string& varName_, std::string& srcLoc_)
      : varName(varName_), srcLoc(srcLoc_) {}

  void print(raw_ostream& O) const {
    errs() << "*) Commit assigned value to " + varName;
    errs() << " at " + srcLoc;
    O << "\n";
  }
};

struct DoubleFlushBug : public BugData {
  std::string varName;
  std::string srcLoc;
  std::string prevLoc;

  DoubleFlushBug(std::string varName_, std::string srcLoc_,
                 std::string prevLoc_)
      : varName(varName_), srcLoc(srcLoc_), prevLoc(prevLoc_) {}

  void print(raw_ostream& O) const {
    errs() << "*) Double flush " + varName;
    errs() << " at " + srcLoc + "\n";
    errs() << "\tFlushed before at " + prevLoc;
    O << "\n";
  }
};

} // namespace llvm
