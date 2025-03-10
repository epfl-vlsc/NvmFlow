#pragma once
#include "Common.h"

namespace llvm {

class FunctionSet {
protected:
  std::unordered_set<Function*> fs;

public:
  FunctionSet() {}
  FunctionSet(std::unordered_set<Function*>& fs_) { fs = std::move(fs_); }

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

  void remove(Function* f) {
    auto it = fs.find(f);
    if (it != fs.end())
      fs.erase(it);
  }

  void insert(Function* f) { fs.insert(f); }

  void extend(FunctionSet& xs) { fs.insert(xs.begin(), xs.end()); }
};

} // namespace llvm