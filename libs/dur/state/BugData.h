#pragma once
#include "Common.h"

namespace llvm {

struct NotCommittedBug {
  static void report(std::string& varName, std::string& prevName,
                     std::string& srcLoc) {
    errs() << "*) For " + varName;
    errs() << " at " + srcLoc + "\n";
    errs() << "\tCommit " + prevName + "\n";
  }
};

struct DoubleFlushBug {
  static void report(std::string& varName, std::string& prevName,
                     std::string& srcLoc, std::string& prevLoc) {
    errs() << "*) Double flush " + varName;
    errs() << " at " + srcLoc + "\n";
    errs() << "\tFlushed before " + prevName;
    errs() << " at " + prevLoc + "\n";
  }
};

struct FinalCommitBug {
  static void report(std::string& varName, std::string& funcName) {
    errs() << "*) \tCommit " + varName;
    errs() << " at the end of " + funcName + "\n";
  }
};

} // namespace llvm
