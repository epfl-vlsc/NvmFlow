#pragma once
#include "Common.h"

namespace llvm {

struct DclValue {
  enum DclState { Write, Flush, Fence, Unseen };
  static const constexpr char* DclStr[] = {"Write", "Flush", "Fence", "Unseen"};

  DclState dcl;

  DclValue(DclState dcl_) : dcl(dcl_) {}

  DclValue(const DclValue& X) : dcl(X.dcl) {}

  DclValue() : dcl(Unseen) {}

  bool operator<(const DclValue& X) const { return dcl < X.dcl; }

  bool operator==(const DclValue& X) const { return dcl == X.dcl; }

  void meetValue(const DclValue& X) {
    if (dcl > X.dcl) {
      dcl = X.dcl;
    }
  }

  auto getName() const {
    auto name = std::string("dcl:") + DclStr[(int)dcl];
    return name;
  }

  void print(raw_ostream& O) const { O << getName(); }
};

class Lattice {
  DclValue dclValue;

public:
  Lattice() {}

  Lattice(const Lattice& X) { *this = X; }

  Lattice meet(const Lattice& X) {
    dclValue.meetValue(X.dclValue);
    return *this;
  }

  static Lattice getInit() { return Lattice(); }

  static Lattice getWrite(Lattice lattice) {
    lattice.dclValue.dcl = DclValue::Write;
    return lattice;
  }

  static Lattice getFlush(Lattice lattice) {
    lattice.dclValue.dcl = DclValue::Flush;
    return lattice;
  }

  static Lattice getFence(Lattice lattice) {
    lattice.dclValue.dcl = DclValue::Fence;
    return lattice;
  }

  bool isWrite() const { return dclValue.dcl == DclValue::Write; }

  bool isFlush() const { return dclValue.dcl == DclValue::Flush; }

  bool isFence() const { return dclValue.dcl == DclValue::Fence; }

  auto getName() const { return dclValue.getName(); }

  void print(raw_ostream& O) const {
    dclValue.print(O);
  }

  bool operator<(const Lattice& X) const {
    return dclValue < X.dclValue;
  }

  bool operator==(const Lattice& X) const {
    return dclValue == X.dclValue;
  }
};

} // namespace llvm