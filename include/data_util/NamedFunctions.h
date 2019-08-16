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
    O << "\n";
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

class FlushFenceFunctions : public NamedFunctions {
public:
  bool sameName(StringRef name) const {
    return name.equals("clflush") || name.equals("flush_range");
  }

  const char* getName() const { return "flush fence"; }
};

class TxbeginFunctions : public NamedFunctions {
public:
  bool sameName(StringRef name) const { return name.equals("tx_begin"); }

  const char* getName() const { return "txbegin"; }
};

class TxendFunctions : public NamedFunctions {
public:
  bool sameName(StringRef name) const { return name.equals("tx_end"); }

  const char* getName() const { return "txend"; }
};

class LoggingFunctions : public NamedFunctions {
public:
  bool sameName(StringRef name) const { return name.equals("tx_log"); }

  const char* getName() const { return "logging"; }
};

} // namespace llvm