#pragma once
#include "Common.h"
#include "FunctionSet.h"

namespace llvm {

class NamedFunctions : public AnnotatedFunctions {
protected:
  virtual bool sameName(StringRef name) const = 0;

  virtual const char* getName() const = 0;

public:
  NamedFunctions(const char* annot_) : AnnotatedFunctions(annot_) {}
  NamedFunctions() : AnnotatedFunctions(nullptr) {}

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
  PfenceFunctions(const char* annot_) : NamedFunctions(annot_) {}
  PfenceFunctions() : NamedFunctions(nullptr) {}

  bool sameName(StringRef name) const { return name.equals("pfence"); }

  const char* getName() const { return "pfence"; }
};

class VfenceFunctions : public NamedFunctions {
public:
  VfenceFunctions(const char* annot_) : NamedFunctions(annot_) {}
  VfenceFunctions() : NamedFunctions(nullptr) {}

  bool sameName(StringRef name) const { return name.equals("vfence"); }

  const char* getName() const { return "vfence"; }
};

class FlushFunctions : public NamedFunctions {
public:
  FlushFunctions(const char* annot_) : NamedFunctions(annot_) {}
  FlushFunctions() : NamedFunctions(nullptr) {}

  bool sameName(StringRef name) const {
    return name.equals("clflushopt") || name.equals("clwb");
  }

  const char* getName() const { return "clflushopt"; }
};

class FlushFenceFunctions : public NamedFunctions {
public:
  FlushFenceFunctions(const char* annot_) : NamedFunctions(annot_) {}
  FlushFenceFunctions() : NamedFunctions(nullptr) {}

  bool sameName(StringRef name) const {
    return name.equals("clflush") || name.equals("flush_range");
  }

  const char* getName() const { return "flush fence"; }
};

class TxBeginFunctions : public NamedFunctions {
public:
  TxBeginFunctions(const char* annot_) : NamedFunctions(annot_) {}
  TxBeginFunctions() : NamedFunctions(nullptr) {}

  bool sameName(StringRef name) const { return name.equals("tx_begin"); }

  const char* getName() const { return "txbegin"; }
};

class TxEndFunctions : public NamedFunctions {
public:
  TxEndFunctions(const char* annot_) : NamedFunctions(annot_) {}
  TxEndFunctions() : NamedFunctions(nullptr) {}

  bool sameName(StringRef name) const { return name.equals("tx_end"); }

  const char* getName() const { return "txend"; }
};

class LoggingFunctions : public NamedFunctions {
public:
  LoggingFunctions(const char* annot_) : NamedFunctions(annot_) {}
  LoggingFunctions() : NamedFunctions(nullptr) {}

  bool sameName(StringRef name) const { return name.equals("tx_log"); }

  const char* getName() const { return "logging"; }
};

} // namespace llvm