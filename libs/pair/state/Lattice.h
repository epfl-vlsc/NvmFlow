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

struct DclFlush {
  enum State { Flush, Write, Unseen };
  static const constexpr char* Str[] = {"Flush", "Write", "Unseen"};

  State state;

  DclFlush(State state_) : state(state_) {}

  DclFlush(const DclFlush& X) : state(X.state) {}

  DclFlush() : state(Unseen) {}

  bool operator<(const DclFlush& X) const { return state < X.state; }

  bool operator==(const DclFlush& X) const { return state == X.state; }

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

struct SclCommit {
  enum State { Write, Fence, Unseen };
  static const constexpr char* Str[] = {"Write", "Fence", "Unseen"};

  State state;

  SclCommit(State state_) : state(state_) {}

  SclCommit(const SclCommit& X) : state(X.state) {}

  SclCommit() : state(Unseen) {}

  bool operator<(const SclCommit& X) const { return state < X.state; }

  bool operator==(const SclCommit& X) const { return state == X.state; }

  void meetValue(const SclCommit& X) {
    if (state > X.state) {
      state = X.state;
    }
  }

  auto getName() const {
    auto name = std::string("scl:") + Str[(int)state];
    return name;
  }

  void print(raw_ostream& O) const { O << getName(); }
};

class Lattice {
  DclCommit dclCommit;
  DclFlush dclFlush;
  SclCommit sclCommit;

public:
  Lattice() {}

  Lattice(const Lattice& X) { *this = X; }

  Lattice meet(const Lattice& X) {
    dclCommit.meetValue(X.dclCommit);
    dclFlush.meetValue(X.dclFlush);
    sclCommit.meetValue(X.sclCommit);
    return *this;
  }

  static Lattice getInit() { return Lattice(); }

  static Lattice getWrite(Lattice lattice) {
    lattice.dclCommit.state = DclCommit::Write;
    lattice.dclFlush.state = DclFlush::Write;
    lattice.sclCommit.state = SclCommit::Write;
    return lattice;
  }

  static Lattice getDclFlushFlush(Lattice lattice) {
    lattice.dclFlush.state = DclFlush::Flush;
    return lattice;
  }

  static Lattice getCommitFlush(Lattice lattice) {
    lattice.dclCommit.state = DclCommit::Flush;
    return lattice;
  }

  static Lattice getCommitFence(Lattice lattice) {
    lattice.dclCommit.state = DclCommit::Fence;
    lattice.sclCommit.state = SclCommit::Fence;
    return lattice;
  }

  static Lattice getPfence(Lattice lattice) {
    lattice.dclCommit.state = DclCommit::Fence;
    return lattice;
  }

  static Lattice getFlushFence(Lattice lattice) {
    lattice.dclCommit.state = DclCommit::Fence;
    lattice.dclFlush.state = DclFlush::Flush;
    lattice.sclCommit.state = SclCommit::Fence;
    return lattice;
  }

  static Lattice getVfence(Lattice lattice) {
    lattice.sclCommit.state = SclCommit::Fence;
    return lattice;
  }

  bool isSclCommitWrite() const { return sclCommit.state == SclCommit::Write; }

  bool isDclCommitWrite() const { return dclCommit.state == DclCommit::Write; }

  bool isDclCommitFlush() const { return dclCommit.state == DclCommit::Flush; }

  bool isDclFlush() const {
    return dclCommit.state == DclCommit::Flush &&
           dclFlush.state == DclFlush::Flush;
  }

  bool isDclFence() const {
    return dclCommit.state == DclCommit::Fence &&
           dclFlush.state == DclFlush::Flush;
  }

  bool isDclFlushFlush() const { return dclFlush.state == DclFlush::Flush; }

  bool isWrite() const {
    return (dclCommit.state == DclCommit::Write &&
            dclFlush.state == DclFlush::Write) &&
           sclCommit.state == SclCommit::Write;
  }

  bool isUnseen() const {
    return dclCommit.state == DclCommit::Unseen ||
           sclCommit.state == SclCommit::Unseen;
  }

  auto getName() const {
    return dclCommit.getName() + " " + dclFlush.getName() + " " +
           sclCommit.getName();
  }

  void print(raw_ostream& O) const {
    dclCommit.print(O);
    O << " ";
    dclFlush.print(O);
    O << " ";
    sclCommit.print(O);
  }

  bool operator<(const Lattice& X) const {
    return dclCommit < X.dclCommit || dclFlush < X.dclFlush ||
           sclCommit < X.sclCommit;
  }

  bool operator==(const Lattice& X) const {
    return dclCommit == X.dclCommit && dclFlush == X.dclFlush &&
           sclCommit == X.sclCommit;
  }
};

} // namespace llvm