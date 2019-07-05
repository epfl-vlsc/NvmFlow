#pragma once
#include "Common.h"

namespace llvm {

class TxValue {
protected:
  using TxState = int;
  TxState tx;

public:
  TxValue(int tx_) : tx(tx_) {}

  TxValue(const TxValue& X) : tx(X.tx) {}

  TxValue() : tx(0) {}

  bool operator<(const TxValue& X) const { return tx < X.tx; }

  bool operator==(const TxValue& X) const { return tx == X.tx; }

  bool inTx() const { return tx > 0; }

  static TxValue getBeginTx(const TxValue& X) {
    TxValue value(X);
    value.tx++;
    return value;
  }

  static TxValue getEndTx(const TxValue& X) {
    TxValue value(X);
    value.tx--;
    return value;
  }

  static TxValue getInit() { return TxValue(); }

  void print(raw_ostream& O) const { O << " tx: " << tx; }
};

class LogValue {
protected:
  enum LogState { Logged, Unseen };
  static const constexpr char* LogStr[] = {"Logged", "Unseen"};

  LogState log;

public:
  LogValue(LogState log_) : log(log_) {}

  LogValue(const LogValue& X) : log(X.log) {}

  LogValue() : log(Unseen) {}

  bool operator<(const LogValue& X) const { return log < X.log; }

  bool operator==(const LogValue& X) const { return log == X.log; }

  bool isLogged() const { return log == Logged; }

  static LogValue getInit() { return LogValue(); }

  static LogValue getLogged() { return LogValue(Logged); }

  void print(raw_ostream& O) const { O << " log: " << LogStr[(int)log]; }
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

  static Lattice getInitLog() { return Lattice(LogType); }

  static Lattice getInitTx() { return Lattice(TxType); }

  static Lattice getLogged() {
    Lattice lattice;
    lattice.val.logValue = LogValue::getLogged();
    return lattice;
  }

  static Lattice getBeginTx(const Lattice& X) {
    assert(X.type == TxType);
    Lattice lattice;
    lattice.val.txValue = TxValue::getBeginTx(X.val.txValue);
    return lattice;
  }

  static Lattice getEndTx(const Lattice& X) {
    assert(X.type == TxType);
    Lattice lattice;
    lattice.val.txValue = TxValue::getBeginTx(X.val.txValue);
    return lattice;
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

  bool isInTx() const {
    assert(type == TxType);
    return val.txValue.inTx();
  }

  bool isLogged() const {
    assert(type == LogType);
    return val.logValue.isLogged();
  }
};

} // namespace llvm