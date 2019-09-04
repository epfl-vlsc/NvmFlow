#pragma once
#include "Common.h"
#include "Traversal.h"
namespace llvm {

template <typename Type> class Worklist {
  std::set<Type> worklist;
  std::list<Type> task;

public:
  void insert(Type e) {
    if (!worklist.count(e)) {
      worklist.insert(e);
      task.push_back(e);
    }
  }

  Type popVal() {
    auto& e = task.front();
    task.pop_front();
    worklist.erase(e);
    return e;
  }

  bool empty() const { return worklist.empty(); }
};

const char* getCIStr(CallInst* ci) {
  if (ci) {
    auto* f = ci->getCalledFunction();
    // data may not be a null terminating ptr
    return f->getName().data();
  } else {
    return "None";
  }
}

class Context {
  CallInst* caller;
  CallInst* callee;

public:
  Context() : caller(nullptr), callee(nullptr) {}
  Context(const Context& X, CallInst* Callee) {
    caller = X.callee;
    callee = Callee;
  }

  bool operator<(const Context& X) const {
    return std::tie(caller, callee) < std::tie(X.caller, X.callee);
  }

  bool operator==(const Context& X) const {
    return (caller == X.caller && callee == X.callee);
  }

  bool operator!=(const Context& X) const {
    return (caller != X.caller || callee != X.callee);
  }

  auto getName() const {
    auto name = std::string("Context:(") + getCIStr(caller) + "," +
                getCIStr(callee) + ")";
    return name;
  }

  auto* getCaller() { return caller; }

  auto* getCallee() { return callee; }

  bool isTop() const { return !caller && !callee; }

  void print(raw_ostream& O) const { O << getName(); }
};

} // namespace llvm