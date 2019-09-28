#pragma once
#include "Common.h"

namespace llvm {

struct TxValue {
  using TxState = int;
  TxState state;

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
  enum LogState { Unseen, Logged };
  static const constexpr char* Str[] = {"Unseen", "Logged"};

  const char* str;
  LogState state;

  LogValue(LogState state_) : state(state_) {}

  LogValue(const LogValue& X) : state(X.state) {}

  LogValue() : state(Unseen) {}

  LogValue(const char* str_) : str(str_), state(Unseen) {}

  bool operator<(const LogValue& X) const { return state < X.state; }

  bool operator==(const LogValue& X) const { return state == X.state; }

  void meetValue(const LogValue& X) {
    if (normal) {
      if (state > X.state) {
        state = X.state;
      }
    } else {
      if (state < X.state) {
        state = X.state;
      }
    }
  }

  auto getName() const {
    std::string name(str);
    name += ":";
    name += Str[(int)state];
    return name;
  }

  void print(raw_ostream& O) const { O << getName(); }
};

} // namespace llvm