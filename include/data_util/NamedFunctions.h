#pragma once
#include "AnnotatedFunctions.h"
#include "Common.h"
#include "FunctionSet.h"
#include "NameFilter.h"

namespace llvm {

// procedure for adding a new tracked function
// 1) update name filter if necessary
// 2) update instr parser if necessary
// 3) update one of the named function below

class NamedFunctions : public AnnotatedFunctions {
protected:
  virtual bool sameName(StringRef name) const = 0;

  virtual const char* getName() const = 0;

public:
  NamedFunctions(const char* annot_) : AnnotatedFunctions(annot_) {}
  NamedFunctions() : AnnotatedFunctions(nullptr) {}

  void addNamedFunc(Function* f, StringRef name) {
    if (sameName(name)) {
      fs.insert(f);
    }
  }

  void print(raw_ostream& O) const {
    O << getName() << ": ";
    FunctionSet::print(O);
    O << "\n";
  }
};

class StoreFunctions : public NamedFunctions {
public:
  StoreFunctions(const char* annot_) : NamedFunctions(annot_) {}
  StoreFunctions() : NamedFunctions(nullptr) {}

  bool sameName(StringRef name) const {
    return name.contains("llvm.memcpy") || name.contains("llvm.memmove");
  }

  const char* getName() const { return "store"; }
};

class PfenceFunctions : public NamedFunctions {
public:
  PfenceFunctions(const char* annot_) : NamedFunctions(annot_) {}
  PfenceFunctions() : NamedFunctions(nullptr) {}

  bool sameName(StringRef name) const { return name.equals("_Z6pfencev"); }

  const char* getName() const { return "pfence"; }
};

class VfenceFunctions : public NamedFunctions {
public:
  VfenceFunctions(const char* annot_) : NamedFunctions(annot_) {}
  VfenceFunctions() : NamedFunctions(nullptr) {}

  bool sameName(StringRef name) const { return name.equals("_Z6vfencev"); }

  const char* getName() const { return "vfence"; }
};

class FlushFunctions : public NamedFunctions {
public:
  FlushFunctions(const char* annot_) : NamedFunctions(annot_) {}
  FlushFunctions() : NamedFunctions(nullptr) {}

  bool sameName(StringRef name) const { return name.equals("_Z8pm_flushPKv"); }

  const char* getName() const { return "flush"; }
};

class FlushFenceFunctions : public NamedFunctions {
public:
  FlushFenceFunctions(const char* annot_) : NamedFunctions(annot_) {}
  FlushFenceFunctions() : NamedFunctions(nullptr) {}

  bool sameName(StringRef name) const {
    return name.equals("_Z13pm_flushfencePKv") || name.equals("flush_range");
  }

  const char* getName() const { return "flush fence"; }
};

class TxBeginFunctions : public NamedFunctions {
public:
  TxBeginFunctions(const char* annot_) : NamedFunctions(annot_) {}
  TxBeginFunctions() : NamedFunctions(nullptr) {}

  bool sameName(StringRef name) const { return name.equals("_Z8tx_beginv"); }

  const char* getName() const { return "tx_begin"; }
};

class TxEndFunctions : public NamedFunctions {
public:
  TxEndFunctions(const char* annot_) : NamedFunctions(annot_) {}
  TxEndFunctions() : NamedFunctions(nullptr) {}

  bool sameName(StringRef name) const { return name.equals("_Z6tx_endv"); }

  const char* getName() const { return "tx_end"; }
};

class LoggingFunctions : public NamedFunctions {
public:
  LoggingFunctions(const char* annot_) : NamedFunctions(annot_) {}
  LoggingFunctions() : NamedFunctions(nullptr) {}

  bool sameName(StringRef name) const {
    return name.equals("_Z6tx_logPv") || name.equals("pmemobj_tx_add_range");
  }

  const char* getName() const { return "tx_log"; }
};

} // namespace llvm