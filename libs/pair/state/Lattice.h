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

struct SclValue {
  enum SclState { Write, Fence, Unseen };
  static const constexpr char* SclStr[] = {"Write", "Fence", "Unseen"};

  SclState scl;

  SclValue(SclState scl_) : scl(scl_) {}

  SclValue(const SclValue& X) : scl(X.scl) {}

  SclValue() : scl(Unseen) {}

  bool operator<(const SclValue& X) const { return scl < X.scl; }

  bool operator==(const SclValue& X) const { return scl == X.scl; }

  void meetValue(const SclValue& X) {
    if (scl > X.scl) {
      scl = X.scl;
    }
  }

  auto getName() const {
    auto name = std::string("scl:") + SclStr[(int)scl];
    return name;
  }

  void print(raw_ostream& O) const { O << getName(); }
};

class Lattice {
  DclValue dclValue;
  SclValue sclValue;

public:
  Lattice() {}

  Lattice(const Lattice& X) { *this = X; }

  Lattice meet(const Lattice& X) {
    dclValue.meetValue(X.dclValue);
    sclValue.meetValue(X.sclValue);
    return *this;
  }

  static Lattice getInit() { return Lattice(); }

  static Lattice getWrite(Lattice lattice) {
    lattice.dclValue.dcl = DclValue::Write;
    lattice.sclValue.scl = SclValue::Write;
    return lattice;
  }

  static Lattice getFlush(Lattice lattice) {
    lattice.dclValue.dcl = DclValue::Flush;
    return lattice;
  }

  static Lattice getPfence(Lattice lattice) {
    lattice.dclValue.dcl = DclValue::Fence;
    return lattice;
  }

  static Lattice getVfence(Lattice lattice) {
    lattice.sclValue.scl = SclValue::Fence;
    return lattice;
  }

  bool isWriteDcl() const {
    return dclValue.dcl == DclValue::Write;
  }

  bool isFlushDcl() const {
    return dclValue.dcl == DclValue::Flush;
  }

  bool isFenceDcl() const {
    return dclValue.dcl == DclValue::Fence;
  }

  bool isWriteScl() const {
    return sclValue.scl == SclValue::Write;
  }

  bool isFenceScl() const {
    return sclValue.scl == SclValue::Fence;
  }

  auto getName() const {
    return dclValue.getName() + " " + sclValue.getName();
  }

  void print(raw_ostream& O) const {
    dclValue.print(O);
    O << " ";
    sclValue.print(O);
  }

  bool operator<(const Lattice& X) const {
    return dclValue < X.dclValue || sclValue < X.sclValue;
  }

  bool operator==(const Lattice& X) const {
    return dclValue == X.dclValue && sclValue == X.sclValue;
  }
};

} // namespace llvm