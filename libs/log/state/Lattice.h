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

  static TxValue getTxbegin(const TxValue& X) {
    TxValue value(X);
    value.tx++;
    return value;
  }

  static TxValue getTxend(const TxValue& X) {
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

  static LogValue getInit() { return LogValue(); }

  static LogValue getLogged() { return LogValue(Logged); }

  void print(raw_ostream& O) const { O << " log: " << LogStr[(int)log]; }
};

} // namespace llvm