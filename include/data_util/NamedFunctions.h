#pragma once
#include "Common.h"
#include "FunctionSet.h"

namespace llvm {

class NamedFunctions : public FunctionSet {
protected:
  virtual bool sameName(StringRef name) const = 0;

  virtual const char* getName() const = 0;

public:
  void insertNamedFunction(Function* f, StringRef realName) {
    if (sameName(realName)) {
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
  bool sameName(StringRef name) const { return name.equals("pfence"); }

  const char* getName() const { return "pfence"; }
};

class VfenceFunctions : public NamedFunctions {
public:
  bool sameName(StringRef name) const { return name.equals("vfence"); }

  const char* getName() const { return "vfence"; }
};

class FlushFunctions : public NamedFunctions {
public:
  bool sameName(StringRef name) const {
    return name.equals("clflushopt") || name.equals("clwb");
  }

  const char* getName() const { return "clflushopt"; }
};

class FlushfenceFunctions : public NamedFunctions {
public:
  bool sameName(StringRef name) const { return name.equals("clflush"); }

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