#pragma once
#include "Common.h"

namespace llvm {

class BugData {
public:
  virtual std::string getName() const = 0;
  virtual ~BugData() {}
};

class NotCommittedBug : public BugData {
  std::string varName;
  std::string prevName;
  std::string srcLoc;
  std::string prevLoc;

public:
  NotCommittedBug(std::string& varName_, std::string& prevName_,
                  std::string& srcLoc_, std::string& prevLoc_)
      : varName(varName_), prevName(prevName_), srcLoc(srcLoc_),
        prevLoc(prevLoc_) {}

  std::string getName() const override {
    std::string name;
    name.reserve(200);

    name += "For " + varName;
    name += " at " + prevName + "\n";
    name += "\tCommit " + srcLoc;
    name += " at " + prevLoc + "\n";

    return name;
  }
};

class DoubleFlushBug : public BugData {
  std::string varName;
  std::string prevName;
  std::string srcLoc;
  std::string prevLoc;

public:
  DoubleFlushBug(std::string& varName_, std::string& prevName_,
                 std::string& srcLoc_, std::string& prevLoc_)
      : varName(varName_), prevName(prevName_), srcLoc(srcLoc_),
        prevLoc(prevLoc_) {}

  std::string getName() const override {
    std::string name;
    name.reserve(200);

    name += "Double flush " + varName;
    name += " at " + prevName + "\n";
    name += "\tFlushed before " + srcLoc;
    name += " at " + prevLoc + "\n";

    return name;
  }
};

class SentinelCommitBug : public BugData {
  std::string varName;
  std::string funcName;

public:
  SentinelCommitBug(std::string& varName_, std::string& funcName_)
      : varName(varName_), funcName(funcName_) {}

  std::string getName() const override {
    std::string name;
    name.reserve(200);

    name += "\tCommit " + varName;
    name += " at the end of " + funcName + "\n";

    return name;
  }
};

class BugFactory {
public:
  static auto* getNotCommittedBug(std::string& varName, std::string& prevName,
                                  std::string& srcLoc, std::string& prevLoc) {
    return new NotCommittedBug{varName, prevName, srcLoc, prevLoc};
  }

  static auto* getDoubleFlushBug(std::string& varName, std::string& prevName,
                                 std::string& srcLoc, std::string& prevLoc) {
    return new DoubleFlushBug{varName, prevName, srcLoc, prevLoc};
  }

  static auto* getSentinelCommitBug(std::string& varName,
                                    std::string& funcName) {
    return new SentinelCommitBug{varName, funcName};
  }
};

} // namespace llvm