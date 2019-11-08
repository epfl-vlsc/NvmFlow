#pragma once
#include "Common.h"

#include "analysis_util/BugUtil.h"

namespace llvm {

struct CommitPairBug : public BugData {
  std::string varName;
  std::string prevName;
  std::string curLoc;
  std::string prevLoc;

  CommitPairBug(std::string varName_, std::string prevName_,
                std::string curLoc_, std::string prevLoc_)
      : varName(varName_), prevName(prevName_), curLoc(curLoc_),
        prevLoc(prevLoc_) {}

  void print(raw_ostream& O) const {
    O << "A) For " + varName;
    O << " at " + curLoc + "\n";
    O << "\tCommit " + prevName;
    O << " at " + prevLoc;
    O << "\n";
  }
};

struct DoubleFlushBug : public BugData {
  std::string varName;
  std::string curLoc;
  std::string prevLoc;

  DoubleFlushBug(std::string varName_, std::string curLoc_,
                 std::string prevLoc_)
      : varName(varName_), curLoc(curLoc_), prevLoc(prevLoc_){}

  void print(raw_ostream& O) const {
    O << "B) Double flush " + varName;
    O << " at " + curLoc + "\n";
    O << "\tFlushed before at " + prevLoc;
    O << "\n";
  }
};

struct VolatileSentinelBug : public BugData {
  std::string varName;
  std::string funcName;

  VolatileSentinelBug(std::string varName_, std::string funcName_)
      : varName(varName_), funcName(funcName_) {}

  void print(raw_ostream& O) const {
    O << "C) Commit " + varName;
    O << " at the end of " + funcName;
    O << "\n";
  }
};

} // namespace llvm
