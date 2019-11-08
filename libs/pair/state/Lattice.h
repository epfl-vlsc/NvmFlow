#pragma once
#include "Common.h"

#include "analysis_util/PersistLattices.h"

namespace llvm {

class Lattice {
  DclCommit dclCommit;
  DclFlush dclFlush;

public:
  Lattice() {}

  Lattice(const Lattice& X) { *this = X; }

  std::pair<int, int> getValuePair() const {
    return std::pair((int)dclCommit.state, (int)dclFlush.state);
  }

  Lattice meet(const Lattice& X) {
    dclCommit.meetValue(X.dclCommit);
    dclFlush.meetValue(X.dclFlush);
    return *this;
  }

  static Lattice getInit() { return Lattice(); }

  static Lattice getWrite(Lattice lattice) {
    lattice.dclCommit.state = DclCommit::Write;
    lattice.dclFlush.state = DclFlush::Write;
    return lattice;
  }

  static Lattice getFlush(Lattice lattice, bool useFence) {
    if (useFence)
      lattice.dclCommit.state = DclCommit::Fence;
    else
      lattice.dclCommit.state = DclCommit::Flush;

    lattice.dclFlush.state = DclFlush::Flush;
    return lattice;
  }

  static Lattice getFence(Lattice lattice) {
    lattice.dclCommit.state = DclCommit::Fence;
    return lattice;
  }

  bool isFlush() const { return dclCommit.state == DclCommit::Flush; }

  bool isFlushed() const { return dclFlush.state == DclFlush::Flush; }

  bool isWriteFlush() const {
    return dclCommit.state == DclCommit::Write ||
           dclCommit.state == DclCommit::Flush;
  }

  bool isUnseen() const { return dclCommit.state == DclCommit::Unseen; }

  bool isDclFlush() const {
    return dclCommit.state == DclCommit::Flush &&
           dclFlush.state == DclFlush::Flush;
  }

  bool isFinal() const {
    return (dclCommit.state == DclCommit::Fence ||
            dclCommit.state == DclCommit::Unseen) &&
           dclFlush.state == DclFlush::Flush;
  }

  bool isWrite() const {
    return dclCommit.state == DclCommit::Write &&
           dclFlush.state == DclFlush::Write;
  }

  bool isWriteCommit() const { return dclCommit.state == DclCommit::Write; }

  auto getName() const {
    return dclCommit.getName() + " " + dclFlush.getName();
  }

  auto getLocInfo() const{
    return std::string("");
  }

  void print(raw_ostream& O) const {
    dclCommit.print(O);
    O << " ";
    dclFlush.print(O);
  }

  bool operator<(const Lattice& X) const {
    return std::tie(dclCommit, dclFlush) < std::tie(X.dclCommit, X.dclFlush);
  }

  bool operator==(const Lattice& X) const {
    return dclCommit == X.dclCommit && dclFlush == X.dclFlush;
  }
};

} // namespace llvm