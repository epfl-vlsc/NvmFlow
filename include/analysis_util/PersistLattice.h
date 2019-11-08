#pragma once
#include "Common.h"

#include "PersistLattices.h"

namespace llvm {

class Lattice {
  DclCommit dclCommit;
  DclFlush dclFlush;

public:
  Lattice() {}

  Lattice(const Lattice& X) { *this = X; }

  Lattice meet(const Lattice& X) {
    dclCommit.meetValue(X.dclCommit);
    dclFlush.meetValue(X.dclFlush);
    return *this;
  }

  static Lattice getInit() { return Lattice(); }

  static Lattice getWrite(Lattice lattice, Instruction* i, const Context& c) {
    lattice.dclCommit.setValue(DclCommit::Write, i, c);
    lattice.dclFlush.setValue(DclFlush::Write, i, c);
    return lattice;
  }

  static Lattice getFlush(Lattice lattice, bool useFence, Instruction* i,
                          const Context& c) {
    auto dclCommitState = (useFence) ? DclCommit::Fence : DclCommit::Flush;

    lattice.dclCommit.setValue(dclCommitState, i, c);
    lattice.dclFlush.setValue(DclFlush::Flush, i, c);
    return lattice;
  }

  static Lattice getFence(Lattice lattice, Instruction* i, const Context& c) {
    lattice.dclCommit.setValue(DclCommit::Fence, i, c);
    return lattice;
  }

  bool isFlush() const { return dclCommit.state == DclCommit::Flush; }

  bool isFlushed() const { return dclFlush.state == DclFlush::Flush; }

  bool isFence() const { return dclCommit.state == DclCommit::Fence; }

  bool isPersistent() const {
    return dclCommit.state == DclCommit::Fence ||
           dclCommit.state == DclCommit::Unseen;
  }

  bool isWrite() const {
    return (dclCommit.state == DclCommit::Write &&
            dclFlush.state == DclFlush::Write);
  }

  bool isWriteCommit() const { return dclCommit.state == DclCommit::Write; }

  bool isDclFlush() const {
    return dclCommit.state == DclCommit::Flush &&
           dclFlush.state == DclFlush::Flush;
  }

  bool isFinal() const {
    return (dclCommit.state == DclCommit::Fence ||
            dclCommit.state == DclCommit::Unseen) &&
           dclFlush.state == DclFlush::Flush;
  }

  bool isWriteFlush() const {
    return dclCommit.state == DclCommit::Write ||
           dclCommit.state == DclCommit::Flush;
  }

  bool isUnseen() const { return dclCommit.state == DclCommit::Unseen; }

  auto getName() const {
    return dclCommit.getName() + " " + dclFlush.getName();
  }

  auto getCommitInfo() const { return dclCommit.getInfo(); }

  auto getFlushInfo() const { return dclFlush.getInfo(); }

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