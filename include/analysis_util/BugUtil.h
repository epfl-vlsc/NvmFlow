#pragma once
#include "Common.h"

#include "DataflowResults.h"
#include "LastSeen.h"

namespace llvm {

struct BugData {
  virtual void print(raw_ostream& O) const = 0;

  virtual ~BugData() {}
};

template <typename Globals, typename LatVar, typename LatVal> class BugUtil {
public:
  using AbstractState = std::map<LatVar, LatVal>;
  using DfResults = DataflowResults<AbstractState>;
  using BugList = std::vector<BugData*>;
  using SeenBugVars = std::set<LatVar>;
  using SeenBugLocs = std::unordered_set<std::string>;
  using LastLoc = LastSeen<LatVar, LatVal>;

protected:
  Globals& globals;
  DfResults& dfResults;
  Function* topFunction;

  BugList bugList;
  LastLoc lastSeen;
  SeenBugVars buggedVars;
  SeenBugLocs buggedLocs;

  void clear() {
    dfResults.clear();
    for (auto* b : bugList)
      delete b;
    bugList.clear();
    lastSeen.clear();
    buggedVars.clear();
    buggedLocs.clear();
  }

  void printTitle(raw_ostream& O) const {
    assert(topFunction);
    auto fncName = globals.dbgInfo.getFunctionName(topFunction);
    O << bugList.size() << " bugs in " << fncName << "\n";
  }

  BugUtil(Globals& globals_, DfResults& dfResults_)
      : globals(globals_), dfResults(dfResults_) {
    topFunction = nullptr;
  }

  virtual ~BugUtil() {}

  void print(raw_ostream& O) const {
    printTitle(O);
    for (auto* bugData : bugList) {
      bugData->print(O);
    }
    O << "\n\n\n";
  }

  void checkEndBug() {
    auto& state = dfResults.getFinalState();
    checkEndBug(state);
  }

  virtual void checkEndBug(AbstractState& state) { (void)(state); }

public:
  void initUnit(Function* function) {
    topFunction = function;
    clear();
  }

  void report() {
    checkEndBug();
    print(errs());
  }

  void addLastSeen(LatVar var, LatVal val, Instruction* i, const Context& c) {
    lastSeen.addLastSeen(var, val, i, c);
  }

  auto& getLastCommit(LatVar var, LatVal val) {
    return lastSeen.getLastCommit(var, val);
  }

  auto& getLastFlush(LatVar var, LatVal val) {
    return lastSeen.getLastFlush(var, val);
  }

  std::string getFunctionName() const {
    return globals.dbgInfo.getFunctionName(topFunction);
  }

  void addBugData(BugData* bugData) { bugList.push_back(bugData); }

  bool isBugVar(LatVar var) const { return buggedVars.count(var); }

  void addBugVar(LatVar var) { buggedVars.insert(var); }

  bool isBugLoc(std::string& loc) const { return buggedLocs.count(loc); }

  void addBugLoc(std::string& loc) { buggedLocs.insert(loc); }
};

} // namespace llvm