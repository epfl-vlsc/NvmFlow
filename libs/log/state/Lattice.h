#pragma once
#include "Common.h"

#include "analysis_util/LogLattices.h"

namespace llvm {

class Lattice {
  enum LatticeType { LogType, TxType, None };
  static const constexpr char* Str[] = {"LogType", "TxType", "None"};
  using LogCommit = LogValue<true>;
  using LogFlush = LogValue<false>;

  LatticeType type;
  LogCommit logCommit;
  LogFlush logFlush;
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
      logFlush.meetValue(X.logFlush);
    } else if (type == TxType) {
      txValue.meetValue(X.txValue);
    } else {
      report_fatal_error("wrong lattice type");
    }

    return *this;
  }

  static Lattice getInitLog() { return Lattice(LogType); }

  static Lattice getInitTx() { return Lattice(TxType); }

  static Lattice getLogged(Instruction* i, const Context& c) {
    Lattice lattice(LogType);
    lattice.logCommit.setValue(LogCommit::Logged, i, c);
    lattice.logFlush.setValue(LogFlush::Logged, i, c);
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
    return logCommit.state == LogCommit::Logged;
  }

  bool isLogged() const {
    assert(type == LogType);
    return logFlush.state == LogFlush::Logged;
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

  auto getCommitInfo() const{
    return logCommit.getInfo();
  }

  auto getFlushInfo() const{
    return logFlush.getInfo();
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