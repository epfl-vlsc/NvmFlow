#pragma once
#include "Common.h"

namespace llvm {

struct TxValue {
  using TxState = int;
  TxState tx;

  TxValue(int tx_) : tx(tx_) {}

  TxValue(const TxValue& X) : tx(X.tx) {}

  TxValue() : tx(0) {}

  bool operator<(const TxValue& X) const { return tx < X.tx; }

  bool operator==(const TxValue& X) const { return tx == X.tx; }

  void meetValue(const TxValue& X) {
    if (tx > X.tx) {
      tx = X.tx;
    }
  }

  auto getName() const {
    auto name = std::string("tx:") + std::to_string(tx);
    return name;
  }

  void print(raw_ostream& O) const { O << getName(); }
};

struct LogValue {
  enum LogState { Logged, Unseen };
  static const constexpr char* LogStr[] = {"Logged", "Unseen"};

  LogState log;

  LogValue(LogState log_) : log(log_) {}

  LogValue(const LogValue& X) : log(X.log) {}

  LogValue() : log(Unseen) {}

  bool operator<(const LogValue& X) const { return log < X.log; }

  bool operator==(const LogValue& X) const { return log == X.log; }

  void meetValue(const LogValue& X) {
    if (log > X.log) {
      log = X.log;
    }
  }

  auto getName() const {
    auto name = std::string("log:") + LogStr[(int)log];
    return name;
  }

  void print(raw_ostream& O) const { O << getName(); }
};

class Lattice {
  enum LatticeType { LogType, TxType, None } type;
  union LatticeValue {
    LogValue logValue;
    TxValue txValue;

    LatticeValue(LatticeType latticeType) {
      if (latticeType == LogType) {
        new (&logValue) LogValue();
      } else if (latticeType == TxType) {
        new (&txValue) TxValue();
      } else {
        report_fatal_error("check lattice type");
      }
    }

    LatticeValue() {}
  } val;

public:
  Lattice() : type(None) {}

  Lattice(const Lattice& X) { *this = X; }

  Lattice(LatticeType latticeType) : type(latticeType), val(latticeType) {}

  Lattice meet(const Lattice& X) {
    assert(X.type == TxType);
    Lattice lattice = *this;
    switch (type) {
    case LogType:
      lattice.val.logValue.meetValue(X.val.logValue);
      break;
    case TxType:
      lattice.val.txValue.meetValue(X.val.txValue);
      break;
    default:
      report_fatal_error("wrong lattice type");
    }

    return lattice;
  }

  static Lattice getInitLog() { return Lattice(LogType); }

  static Lattice getInitTx() { return Lattice(TxType); }

  static Lattice getLogged() {
    Lattice lattice(LogType);
    lattice.val.logValue.log = LogValue::Logged;
    return lattice;
  }

  static Lattice getBeginTx(const Lattice& X) {
    assert(X.type == TxType);
    Lattice lattice(X);
    lattice.val.txValue.tx += 1;
    return lattice;
  }

  static Lattice getEndTx(const Lattice& X) {
    assert(X.type == TxType);
    Lattice lattice(X);
    lattice.val.txValue.tx -= 1;
    return lattice;
  }

  bool inTx() const {
    assert(type == TxType);
    return val.txValue.tx > 0;
  }

  bool isLogged() const {
    assert(type == LogType);
    return val.logValue.log == LogValue::Logged;
  }

  auto getName() const {
    if (type == LogType) {
      return val.logValue.getName();
    } else if (type == TxType) {
      return val.txValue.getName();
    } else {
      report_fatal_error("check lattice type");
      return std::string("");
    }
  }

  void print(raw_ostream& O) const {
    if (type == LogType) {
      val.logValue.print(O);
    } else if (type == TxType) {
      val.txValue.print(O);
    } else {
      report_fatal_error("check lattice type");
    }
  }

  bool operator<(const Lattice& X) const {
    if (type == LogType) {
      return val.logValue < X.val.logValue;
    } else if (type == TxType) {
      return val.txValue < X.val.txValue;
    } else {
      report_fatal_error("check lattice type");
    }
  }

  bool operator==(const Lattice& X) const {
    if (type == LogType) {
      return val.logValue == X.val.logValue;
    } else if (type == TxType) {
      return val.txValue == X.val.txValue;
    } else {
      report_fatal_error("check lattice type");
    }
  }
};

} // namespace llvm