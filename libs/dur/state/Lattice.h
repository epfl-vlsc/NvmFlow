#pragma once
#include "Common.h"

namespace llvm {

struct DclCommit {
  enum State { Write, Flush, Fence, Unseen };
  static const constexpr char* Str[] = {"Write", "Flush", "Fence", "Unseen"};

  State state;

  DclCommit(State state_) : state(state_) {}

  DclCommit(const DclCommit& X) : state(X.state) {}

  DclCommit() : state(Unseen) {}

  bool operator<(const DclCommit& X) const { return state < X.state; }

  bool operator==(const DclCommit& X) const { return state == X.state; }

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

class Lattice {
  DclCommit dclCommit;

public:
  Lattice() {}

  Lattice(const Lattice& X) { *this = X; }

  Lattice meet(const Lattice& X) {
    dclCommit.meetValue(X.dclCommit);
    return *this;
  }

  static Lattice getInit() { return Lattice(); }

  static Lattice getWrite(Lattice lattice) {
    lattice.dclCommit.state = DclCommit::Write;
    return lattice;
  }

  static Lattice getFlush(Lattice lattice) {
    lattice.dclCommit.state = DclCommit::Flush;
    return lattice;
  }

  static Lattice getFence(Lattice lattice) {
    lattice.dclCommit.state = DclCommit::Fence;
    return lattice;
  }

  static Lattice getFlushFence(Lattice lattice) {
    lattice.dclCommit.state = DclCommit::Fence;
    return lattice;
  }

  bool isWrite() const { return dclCommit.state == DclCommit::Write; }

  bool isFlush() const { return dclCommit.state == DclCommit::Flush; }

  bool isFence() const { return dclCommit.state == DclCommit::Fence; }

  auto getName() const { return dclCommit.getName(); }

  void print(raw_ostream& O) const { dclCommit.print(O); }

  bool operator<(const Lattice& X) const { return dclCommit < X.dclCommit; }

  bool operator==(const Lattice& X) const { return dclCommit == X.dclCommit; }
};

} // namespace llvm