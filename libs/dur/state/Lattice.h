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

struct DclFlush {
  enum State { Flush, Write, Unseen };
  static const constexpr char* Str[] = {"Flush", "Write", "Unseen"};

  State state;

  DclFlush(State state_) : state(state_) {}

  DclFlush(const DclFlush& X) : state(X.state) {}

  DclFlush() : state(Unseen) {}

  bool operator<(const DclFlush& X) const { return state < X.state; }

  bool operator==(const DclFlush& X) const { return state == X.state; }

  bool operator!=(const DclFlush& X) const { return state != X.state; }

  void meetValue(const DclFlush& X) {
    if (state > X.state) {
      state = X.state;
    }
  }

  auto getName() const {
    auto name = std::string("flush:") + Str[(int)state];
    return name;
  }

  void print(raw_ostream& O) const { O << getName(); }
};

class Lattice {
  DclCommit dclCommit;
  DclFlush dclFlush;

public:
  Lattice() {}

  Lattice(const Lattice& X) { *this = X; }

  Lattice meet(const Lattice& X) {
    dclCommit.meetValue(X.dclCommit);
    dclFlush.meetValue(X.dclFlush);
    return *this;
  }

  static Lattice getInit() { return Lattice(); }

  static Lattice getWrite(Lattice lattice) {
    lattice.dclCommit.state = DclCommit::Write;
    lattice.dclFlush.state = DclFlush::Write;
    return lattice;
  }

  static Lattice getFlush(Lattice lattice, bool useFence) {
    if (useFence)
      lattice.dclCommit.state = DclCommit::Fence;
    else
      lattice.dclCommit.state = DclCommit::Flush;

    lattice.dclFlush.state = DclFlush::Flush;
    return lattice;
  }

  static Lattice getFence(Lattice lattice) {
    lattice.dclCommit.state = DclCommit::Fence;
    return lattice;
  }

  bool isFlush() const { return dclCommit.state == DclCommit::Flush; }

  bool isFlushed() const { return dclFlush.state == DclFlush::Flush; }

  bool isFence() const { return dclCommit.state == DclCommit::Fence; }

  bool isWrite() const {
    return (dclCommit.state == DclCommit::Write &&
            dclFlush.state == DclFlush::Write);
  }

  bool isWriteFlush() const {
    return dclCommit.state == DclCommit::Write ||
           dclCommit.state == DclCommit::Flush;
  }

  bool isUnseen() const { return dclCommit.state == DclCommit::Unseen; }

  auto getName() const {
    return dclCommit.getName() + " " + dclFlush.getName();
  }

  void print(raw_ostream& O) const {
    dclCommit.print(O);
    O << " ";
    dclFlush.print(O);
  }

  bool operator<(const Lattice& X) const {
    return std::tie(dclCommit, dclFlush) < std::tie(X.dclCommit, X.dclFlush);
  }

  bool operator==(const Lattice& X) const {
    return dclCommit == X.dclCommit && dclFlush == X.dclFlush;
  }

  friend bool sameDclCommit(const Lattice& X, const Lattice& Y);

  friend bool sameDclFlush(const Lattice& X, const Lattice& Y);

  friend bool sameSclCommit(const Lattice& X, const Lattice& Y);
};

bool sameDclCommit(const Lattice& X, const Lattice& Y) {
  return X.dclCommit == Y.dclCommit;
}

bool sameDclFlush(const Lattice& X, const Lattice& Y) {
  return X.dclFlush == Y.dclFlush;
}

} // namespace llvm