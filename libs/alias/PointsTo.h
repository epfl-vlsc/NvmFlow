#pragma once
#include "Common.h"
#include "llvm/Analysis/CFLSteensAliasAnalysis.h"

namespace llvm {

class PointsTo {
  std::unique_ptr<CFLSteensAAResult> Result;

public:
  PointsTo(TargetLibraryInfo& TLI){
    Result.reset(new CFLSteensAAResult(TLI));
  }

  CFLSteensAAResult& getResult() { return *Result; }
  const CFLSteensAAResult& getResult() const { return *Result; }
};

} // end namespace llvm
