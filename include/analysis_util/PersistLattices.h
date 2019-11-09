#pragma once
#include "Common.h"
#include "DfUtil.h"

namespace llvm {

struct DclCommit {
  enum State { Write, Flush, Fence, Unseen };
  static const constexpr char* Str[] = {"Write", "Flush", "Fence", "Unseen"};

  State state;
  Instruction* i;
  Context c;

  void setValue(State state_, Instruction* i_, const Context& c_) {
    state = state_;
    i = i_;
    c = c_;
  }

  DclCommit(State state_, Instruction* i_, const Context& c_)
      : state(state_), i(i_), c(c_) {}

  DclCommit(const DclCommit& X) : state(X.state), i(X.i), c(X.c) {}

  DclCommit() : state(Unseen), i(nullptr) {}

  bool operator<(const DclCommit& X) const { return std::tie(state, i, c) < std::tie(X.state, X.i, X.c); }

  bool operator==(const DclCommit& X) const { return std::tie(state, i, c) == std::tie(X.state, X.i, X.c); }

  void meetValue(const DclCommit& X) {
    if (state >= X.state) {
      state = X.state;
      i = X.i;
      c = X.c;
    }
  }

  auto getName() const {
    auto name = std::string("commit:") + Str[(int)state];
    return name;
  }

  auto getInfo() const {
    if (i) {
      auto srcLoc = DbgInstr::getSourceLocation(i);
      return c.getFullName(srcLoc);
    }

    return std::string("");
  }

  void print(raw_ostream& O) const { O << getName(); }
};

struct DclFlush {
  enum State { Flush, Write, Unseen };
  static const constexpr char* Str[] = {"Flush", "Write", "Unseen"};

  State state;
  Instruction* i;
  Context c;

  void setValue(State state_, Instruction* i_, const Context& c_) {
    state = state_;
    i = i_;
    c = c_;
  }

  DclFlush(State state_, Instruction* i_, const Context& c_)
      : state(state_), i(i_), c(c_) {}

  DclFlush(const DclFlush& X) : state(X.state), i(X.i), c(X.c) {}

  DclFlush() : state(Unseen), i(nullptr) {}

  bool operator<(const DclFlush& X) const { return std::tie(state, i, c) < std::tie(X.state, X.i, X.c);}

  bool operator==(const DclFlush& X) const { return std::tie(state, i, c) == std::tie(X.state, X.i, X.c); }

  void meetValue(const DclFlush& X) {
    if (state >= X.state) {
      state = X.state;
      i = X.i;
      c = X.c;
    }
  }

  auto getName() const {
    auto name = std::string("flush:") + Str[(int)state];
    return name;
  }

  auto getInfo() const {
    if (i) {
      auto srcLoc = DbgInstr::getSourceLocation(i);
      return c.getFullName(srcLoc);
    }

    return std::string("");
  }

  void print(raw_ostream& O) const { O << getName(); }
};

} // namespace llvm