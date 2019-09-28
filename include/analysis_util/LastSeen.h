#pragma once
#include "Common.h"

namespace llvm {

template <typename LatVar, typename LatVal> class LastSeen {
public:
  using VarState = std::pair<LatVar, int>;
  using LastInst = std::map<VarState, Instruction*>;

private:
  LastInst commit;
  LastInst flush;

public:
  void addLastSeen(LatVar var, LatVal val, Instruction* i) {
    auto [commitVal, flushVal] = val.getValuePair();
    VarState commitState = {var, commitVal};
    commit[commitState] = i;
    VarState flushState = {var, flushVal};
    flush[flushState] = i;
  }

  Instruction* getLastCommit(LatVar var, LatVal val) {
    auto [commitVal, _] = val.getValuePair();
    VarState commitState = {var, commitVal};
    if (commit.count(commitState))
      return commit[commitState];

    report_fatal_error("not seen var val");
    return nullptr;
  }

  Instruction* getLastFlush(LatVar var, LatVal val) {
    auto [_, flushVal] = val.getValuePair();
    VarState flushState = {var, flushVal};
    if (flush.count(flushState))
      return flush[flushState];

    report_fatal_error("not seen var val");
    return nullptr;
  }

  void clear() {
    commit.clear();
    flush.clear();
  }
};

} // namespace llvm