#pragma once
#include "Common.h"
#include "DfUtil.h"

namespace llvm {

struct InstCntxt {
  Instruction* i;
  Context c;

  InstCntxt(Instruction* i_, Context c_) : i(i_), c(c_) {}

  InstCntxt() : i(nullptr) {}

  auto* getInst() {
    assert(i);
    return i;
  }

  auto getLoc() const {
    assert(i);
    return DbgInstr::getSourceLocation(i);
  }

  auto getName() const {
    assert(i);
    std::string name;
    name.reserve(200);
    name += c.getFullName() + " ";
    name += DbgInstr::getSourceLocation(i);
    return name;
  }
};

template <typename LatVar, typename LatVal> class LastSeen {
public:
  using VarState = std::pair<LatVar, int>;
  using InstCntxtList = std::vector<InstCntxt>;
  using LastInst = std::map<VarState, InstCntxtList>;

private:
  LastInst commit;
  LastInst flush;

  InstCntxt empty;

public:
  void addLastSeen(LatVar var, LatVal val, Instruction* i, const Context& c) {
    InstCntxt instContext = {i, c};

    auto [commitVal, flushVal] = val.getValuePair();

    VarState commitState = {var, commitVal};
    commit[commitState].push_back(instContext);

    VarState flushState = {var, flushVal};
    flush[flushState].push_back(instContext);
  }

  auto& getLastCommit(LatVar var, LatVal val) {
    auto [commitVal, _] = val.getValuePair();
    VarState commitState = {var, commitVal};
    if (commit.count(commitState)) {
      auto& icList = commit[commitState];
      return icList.back();
    }

    report_fatal_error("last commit:not seen var val");
    return empty;
  }

  auto& getLastFlush(LatVar var, LatVal val) {
    auto [_, flushVal] = val.getValuePair();
    VarState flushState = {var, flushVal};
    if (flush.count(flushState)) {
      auto& icList = flush[flushState];
      return icList.back();
    }

    report_fatal_error("last flush:not seen var val");
    return empty;
  }

  void clear() {
    commit.clear();
    flush.clear();
  }
};

} // namespace llvm