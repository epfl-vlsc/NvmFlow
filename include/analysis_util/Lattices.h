#pragma once
#include "Common.h"

namespace llvm {

struct DclCommit {
  enum State { Write, Flush, Fence, Unseen };
  static const constexpr char* Str[] = {"Write", "Flush", "Fence", "Unseen"};

  State state;

  DclCommit(State state_) : state(state_) {}

  DclCommit(const DclCommit& X) { *this = X; }

  DclCommit() : state(Unseen) {}

  bool operator<(const DclCommit& X) const { return state < X.state; }

  bool operator==(const DclCommit& X) const { return state == X.state; }

  bool operator!=(const DclCommit& X) const { return state != X.state; }

  void meetValue(const DclCommit& X) {
    if (state > X.state) {
      state = X.state;
    }
  }

  auto getName() const {
    auto name = std::string("commit:") + Str[(int)state];
    return name;
  }

  void print(raw_ostream& O) const { O << getName(); }
};

template <int LastState> struct ChainLattice {
  static const constexpr char* Str[] = {"Write", "Flush", "Fence", "Unseen"};

  const char* latticeName;
  const char* StateNames;
  int state;

  ChainLattice(int state_) : state(state_) {}

  ChainLattice(const ChainLattice& X) : state(X.state) {}

  ChainLattice() : state(LastState) {}

  bool operator<(const DclCommit& X) const { return state < X.state; }

  bool operator==(const DclCommit& X) const { return state == X.state; }

  bool operator!=(const DclCommit& X) const { return state != X.state; }

  void meetValue(const DclCommit& X) {
    if (state > X.state) {
      state = X.state;
    }
  }

  auto getName() const {
    auto name = std::string("commit:") + Str[(int)state];
    return name;
  }

  void print(raw_ostream& O) const { O << getName(); }
};

} // namespace llvm