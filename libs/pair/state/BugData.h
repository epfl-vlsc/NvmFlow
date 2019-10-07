#pragma once
#include "Common.h"

#include "analysis_util/BugUtil.h"

namespace llvm {

struct CommitPairBug : public BugData {
  std::string varName;
  std::string prevName;
  std::string srcLoc;
  std::string prevLoc;

  CommitPairBug(std::string varName_, std::string prevName_,
                std::string srcLoc_, std::string prevLoc_)
      : varName(varName_), prevName(prevName_), srcLoc(srcLoc_),
        prevLoc(prevLoc_) {}

  void print(raw_ostream& O) const {
    O << "Type 1) For " + varName;
    O << " at " + srcLoc + "\n";
    O << "\tCommit " + prevName;
    O << " at " + prevLoc + "\n";
  }
};

struct CommitDataBug : public BugData {
  std::string varName;
  std::string prevName;
  std::string srcLoc;

  CommitDataBug(std::string varName_, std::string prevName_,
                std::string srcLoc_)
      : varName(varName_), prevName(prevName_), srcLoc(srcLoc_) {}

  void print(raw_ostream& O) const {
    O << "Type 2) For " + varName;
    O << " at " + srcLoc + "\n";
    O << "\tCommit " + prevName + "\n";
  }
};

struct WriteVarBug : public BugData {
  std::string varName;
  std::string srcLoc;

  WriteVarBug(std::string varName_, std::string srcLoc_)
      : varName(varName_), srcLoc(srcLoc_) {}

  void print(raw_ostream& O) const {
    O << "Type 3) \tWrite to " + varName;
    O << " before flushing at " + srcLoc + "\n";
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
    O << "Type 4) Double flush " + varName;
    O << " at " + srcLoc + "\n";
    O << "\tFlushed before at " + prevLoc + "\n";
  }
};

struct VolatileSentinelBug : public BugData {
  std::string varName;
  std::string funcName;

  VolatileSentinelBug(std::string varName_, std::string funcName_)
      : varName(varName_), funcName(funcName_) {}

  void print(raw_ostream& O) const {
    O << "Type 5) Commit " + varName;
    O << " at the end of " + funcName + "\n";
  }
};

} // namespace llvm
