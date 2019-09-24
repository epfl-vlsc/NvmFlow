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

struct LogValue {
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

  void meetValue(const LogValue& X, bool normal = true) {
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

class Lattice {
  enum LatticeType { LogType, TxType, None };
  static const constexpr char* Str[] = {"LogType", "TxType", "None"};

  LatticeType type;
  LogValue logCommit;
  LogValue logFlush;
  TxValue txValue;

public:
  Lattice() : type(None), logCommit("commit"), logFlush("flush") {}

  Lattice(const Lattice& X) { *this = X; }

  Lattice(LatticeType latticeType)
      : type(latticeType), logCommit("commit"), logFlush("flush") {}

  Lattice meet(const Lattice& X) {
    assert(type == X.type);
    if (type == LogType) {
      logCommit.meetValue(X.logCommit);
      logFlush.meetValue(X.logFlush, false);
    } else if (type == TxType) {
      txValue.meetValue(X.txValue);
    } else {
      report_fatal_error("wrong lattice type");
    }

    return *this;
  }

  static Lattice getInitLog() { return Lattice(LogType); }

  static Lattice getInitTx() { return Lattice(TxType); }

  static Lattice getLogged() {
    Lattice lattice(LogType);
    lattice.logCommit.state = LogValue::Logged;
    lattice.logFlush.state = LogValue::Logged;
    return lattice;
  }

  static Lattice getBeginTx(Lattice lattice) {
    assert(lattice.type == TxType);
    lattice.txValue.state += 1;
    return lattice;
  }

  static Lattice getEndTx(Lattice lattice) {
    assert(lattice.type == TxType);
    lattice.txValue.state -= 1;
    return lattice;
  }

  bool inTx() const {
    assert(type == TxType);
    return txValue.state > 0;
  }

  bool isLog() const {
    assert(type == LogType);
    return logCommit.state == LogValue::Logged;
  }

  bool isLogged() const {
    assert(type == LogType);
    return logFlush.state == LogValue::Logged;
  }

  auto getName() const {
    if (type == LogType) {
      return logCommit.getName() + " " + logFlush.getName();
    } else if (type == TxType) {
      return txValue.getName();
    } else {
      report_fatal_error("check lattice type");
      return std::string("");
    }
  }

  void print(raw_ostream& O) const {
    if (type == LogType) {
      logCommit.print(O);
      O << " ";
      logFlush.print(O);
    } else if (type == TxType) {
      txValue.print(O);
    } else {
      report_fatal_error("check lattice type");
    }
  }

  bool operator<(const Lattice& X) const {
    if (type == LogType) {
      return logCommit < X.logCommit || logFlush < X.logFlush;
    } else if (type == TxType) {
      return txValue < X.txValue;
    } else {
      report_fatal_error("check lattice type");
    }
  }

  bool operator==(const Lattice& X) const {
    if (type == LogType) {
      return logCommit == X.logCommit && logFlush == X.logFlush;
    } else if (type == TxType) {
      return txValue == X.txValue;
    } else {
      report_fatal_error("check lattice type");
    }
  }
};

} // namespace llvm