#pragma once
#include "Common.h"

namespace llvm {

struct NotCommittedBug {
  static void report(std::string& varName, std::string& srcLoc) {
    errs() << "*) Commit assigned value to " + varName;
    errs() << " at " + srcLoc + "\n";
  }
};

struct DoubleFlushBug {
  static void report(std::string& varName, std::string& srcLoc,
                     std::string& prevLoc) {
    errs() << "*) Double flush " + varName;
    errs() << " at " + srcLoc + "\n";
    errs() << "\tFlushed before at " + prevLoc + "\n";
  }
};

struct FinalCommitBug {
  static void report(std::string& varName, std::string& funcName) {
    errs() << "*) \tCommit " + varName;
    errs() << " at the end of " + funcName + "\n";
  }
};

} // namespace llvm
