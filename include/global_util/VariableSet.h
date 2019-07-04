#pragma once
#include "Common.h"
#include "VariableInfo.h"

namespace llvm {

class VariableSet {
protected:
  std::set<VariableInfo*> vs;

public:
  void insert(VariableInfo* v) { vs.insert(v); }

  auto begin() { return vs.begin(); }

  auto end() { return vs.end(); }

  void print(raw_ostream& O) const {
    for (auto v : vs) {
      O << v->getName() << ", ";
    }
  }

  auto empty() const { return vs.empty(); }

  auto count(VariableInfo* v) const { return vs.count(v); }

  auto size() const { return vs.size(); }
};

} // namespace llvm