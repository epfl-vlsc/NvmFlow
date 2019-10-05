#pragma once
#include "Common.h"

namespace llvm {

struct DclCommit {
  enum State { Write, Unseen, Flush, Fence };
  static const constexpr char* Str[] = {"Write", "Unseen", "Flush", "Fence"};

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
  enum State { Flush, Unseen, Write };
  static const constexpr char* Str[] = {"Flush", "Unseen", "Write"};

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

struct SclCommit {
  enum State { Write, Unseen, Fence };
  static const constexpr char* Str[] = {"Write", "Unseen", "Fence"};

  State state;

  SclCommit(State state_) : state(state_) {}

  SclCommit(const SclCommit& X) : state(X.state) {}

  SclCommit() : state(Unseen) {}

  bool operator<(const SclCommit& X) const { return state < X.state; }

  bool operator==(const SclCommit& X) const { return state == X.state; }

  bool operator!=(const SclCommit& X) const { return state != X.state; }

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


} // namespace llvm