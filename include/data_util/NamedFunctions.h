#pragma once
#include "AnnotatedFunctions.h"
#include "Common.h"
#include "FunctionSet.h"

namespace llvm {

// add name to here and respective struct
class NameFilter {
  static constexpr const char* fncNames[] = {"_Z6pfencev", "_Z6vfencev",
                                             "_Z8tx_beginv", "_Z6tx_endv"};

  static constexpr const char* varCalls[] = {
      "_Z8pm_flushPKv",       "_Z13pm_flushfencePKv", "flush_range",
      "pmemobj_tx_add_range", "_Z6tx_logPv",          "llvm.memcpy"};

  static constexpr const char* storeFunctions[] = {"llvm.memcpy"};

  static constexpr const size_t ElementSize = sizeof(const char*);

public:
  static bool contains(CallInst* ci, const char* const* names, size_t size) {
    auto* f = ci->getCalledFunction();
    if (!f)
      return false;

    auto n = f->getName();
    if (n.empty())
      return false;

    for (size_t i = 0; i < size; ++i) {
      auto* name = names[i];
      if (n.contains(name)) {
        return true;
      }
    }
    return false;
  }

  static bool isVarCall(CallInst* ci) {
    return contains(ci, varCalls, sizeof(varCalls) / ElementSize);
  }

  static bool isStoreFunction(CallInst* ci) {
    return contains(ci, storeFunctions, sizeof(storeFunctions) / ElementSize);
  }
};

class NamedFunctions : public AnnotatedFunctions {
protected:
  virtual bool sameName(StringRef name) const = 0;

  virtual const char* getName() const = 0;

public:
  NamedFunctions(const char* annot_) : AnnotatedFunctions(annot_) {}
  NamedFunctions() : AnnotatedFunctions(nullptr) {}

  void insertNamedFunction(Function* f, StringRef name) {
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