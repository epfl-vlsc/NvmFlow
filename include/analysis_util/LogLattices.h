#pragma once
#include "Common.h"
#include "DfUtil.h"

namespace llvm {

struct TxValue {
  using State = int;
  State state;

  TxValue(int state_) : state(state_) {}

  TxValue(const TxValue& X) : state(X.state) {}

  TxValue() : state(0) {}

  bool operator<(const TxValue& X) const { return state < X.state; }

  bool operator==(const TxValue& X) const { return state == X.state; }

  void meetValue(const TxValue& X) {
    if (state > X.state) {
      state = X.state;
    }
  }

  auto getName() const {
    auto name = std::string("tx:") + std::to_string(state);
    return name;
  }

  void print(raw_ostream& O) const { O << getName(); }
};

template <bool normal> struct LogValue {
  enum State { Unseen, Logged };
  static const constexpr char* Str[] = {"Unseen", "Logged"};

  const char* str;
  State state;
  Instruction* i;
  Context c;

  void setValue(State state_, Instruction* i_, const Context& c_){
    state = state_;
    i = i_;
    c = c_;
  }

  LogValue(State state_, Instruction* i_, const Context& c_) : state(state_), i(i_), c(c_) {}

  LogValue(const LogValue& X) : state(X.state), i(X.i), c(X.c) {}

  LogValue() : state(Unseen), i(nullptr) {}

  LogValue(const char* str_) : str(str_), state(Unseen), i(nullptr) {}

  bool operator<(const LogValue& X) const { return state < X.state; }

  bool operator==(const LogValue& X) const { return state == X.state; }

  void meetValue(const LogValue& X) {
    if (normal) {
      if (state > X.state) {
        state = X.state;
        i = X.i;
        c = X.c;
      }
    } else {
      if (state < X.state) {
        state = X.state;
        i = X.i;
        c = X.c;
      }
    }
  }

  auto getName() const {
    std::string name(str);
    name += ":";
    name += Str[(int)state];
    return name;
  }

  auto getInfo() const {
    if(i){
      auto srcLoc = DbgInstr::getSourceLocation(i);
      return c.getFullName(srcLoc);
    }

    return std::string("");
  }

  void print(raw_ostream& O) const { O << getName(); }
};

} // namespace llvm