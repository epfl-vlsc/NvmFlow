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

  size_t size() const { return worklist.size(); }
};

const char* getCIStr(CallBase* ci) {
  if (ci) {
    auto* f = getCalledFunction(ci);
    // data may not be a null terminating ptr
    return f->getName().data();
  } else {
    return "None";
  }
}

class Context {
  CallBase* caller;
  CallBase* callee;

  std::vector<CallBase*> callStack;

public:
  Context() : caller(nullptr), callee(nullptr) {}

  Context(const Context& X, CallBase* Callee) {
    caller = X.callee;
    callee = Callee;

    callStack = X.callStack;
    callStack.push_back(callee);
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

  auto getFullName() const {
    std::string name("top/");
    name.reserve(200);

    for (auto* cb : callStack) {
      auto* f = getCalledFunction(cb);
      name += f->getName().str();
      name += "/";
    }

    return name;
  }

  auto* getCaller() { return caller; }

  auto* getCallee() { return callee; }

  bool isTop() const { return !caller && !callee; }

  void print(raw_ostream& O) const { O << getName(); }
};

} // namespace llvm