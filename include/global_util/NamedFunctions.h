#pragma once
#include "Common.h"
#include "FunctionSet.h"

namespace llvm {

class NamedFunctions : public FunctionSet {
protected:
  virtual bool sameName(StringRef name) const = 0;

  virtual const char* getName() const = 0;

public:
  void insertNamedFunction(Function* f) {
    StringRef name = f->getName();
    if (sameName(name)) {
      fs.insert(f);
    }
  }

  void print(raw_ostream& O) const {
    O << getName() << ": ";
    FunctionSet::print(O);
  }
};

class PfenceFunctions : public NamedFunctions {
public:
  bool sameName(StringRef name) const { return name.equals("Z6pfencev"); }

  const char* getName() const { return "pfence"; }
};

class VfenceFunctions : public NamedFunctions {
public:
  bool sameName(StringRef name) const { return name.equals("Z6vfencev"); }

  const char* getName() const { return "vfence"; }
};

class FlushFunctions : public NamedFunctions {
public:
  bool sameName(StringRef name) const {
    return name.equals("Z10clflushoptPKv");
  }

  const char* getName() const { return "flush"; }
};

class FlushfenceFunctions : public NamedFunctions {
public:
  bool sameName(StringRef name) const { return name.equals("Z7clflushPKv"); }

  const char* getName() const { return "flushfence"; }
};

class TxbeginFunctions : public NamedFunctions {
public:
  bool sameName(StringRef name) const { return name.equals("_Z8tx_beginv"); }

  const char* getName() const { return "txbegin"; }
};

class TxendFunctions : public NamedFunctions {
public:
  bool sameName(StringRef name) const { return name.equals("_Z6tx_endv"); }

  const char* getName() const { return "txend"; }
};

class LoggingFunctions : public NamedFunctions {
public:
  bool sameName(StringRef name) const { return name.equals("_Z3logPv"); }

  const char* getName() const { return "logging"; }
};

} // namespace llvm