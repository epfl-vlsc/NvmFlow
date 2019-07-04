#pragma once
#include "Common.h"

namespace llvm {

class FunctionSet {
protected:
  std::set<Function*> fs;

public:
  void insert(Function* f) { fs.insert(f); }

  auto begin() { return fs.begin(); }

  auto end() { return fs.end(); }

  void print(raw_ostream& O) const {
    for (auto f : fs) {
      O << f->getName() << ", ";
    }
  }

  auto empty() const { return fs.empty(); }

  auto count(Function* f) const { return fs.count(f); }

  auto size() const { return fs.size(); }
};

} // namespace llvm