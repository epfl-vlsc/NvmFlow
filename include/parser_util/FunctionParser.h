#pragma once
#include "Common.h"
#include "analysis_util/Traversal.h"

namespace llvm {

template <typename Globals> class FunctionParser {
  static constexpr const char* GLOBAL_ANNOT = "llvm.global.annotations";

  void addAnnotFuncs() {
    for (Module::global_iterator I = M.global_begin(), E = M.global_end();
         I != E; ++I) {
      if (I->getName() == GLOBAL_ANNOT) {
        ConstantArray* CA = dyn_cast<ConstantArray>(I->getOperand(0));
        for (auto OI = CA->op_begin(); OI != CA->op_end(); ++OI) {
          ConstantStruct* CS = dyn_cast<ConstantStruct>(OI->get());
          GlobalVariable* AnnotationGL =
              dyn_cast<GlobalVariable>(CS->getOperand(1)->getOperand(0));
          StringRef annotation =
              dyn_cast<ConstantDataArray>(AnnotationGL->getInitializer())
                  ->getAsCString();

          Function* annotatedFunction =
              dyn_cast<Function>(CS->getOperand(0)->getOperand(0));

          globals.functions.addAnnotFunc(annotatedFunction, annotation);
        }
      }
    }
  }

  void addNamedFuncs() {
    for (auto& F : M) {
      globals.functions.addNamedFunc(&F);
    }
  }

  template <typename FunctionMap>
  void addAllAnalyzed(Function* f, FunctionMap& fm) {
    if (fm.count(f))
      return;

    fm[f];

    for (auto& I : instructions(*f)) {
      if (auto* cb = dyn_cast<CallBase>(&I)) {
        auto* callee = getCalledFunction(cb);

        bool notSeen = !fm.count(callee);
        bool notSkipped = !globals.functions.skipFunction(callee);
        if (notSeen && notSkipped) {
          addAllAnalyzed(callee, fm);
        }
      }
    }
  }

  template <typename FunctionMap>
  void worklistFillAllAnalyzed(FunctionMap& fm) {
    Worklist<Function*> worklist;
    std::unordered_map<Function*, int> change;

    for (auto& [f, _] : fm) {
      worklist.insert(f);
      change[f] = 0;
      auto& fs = fm[f];
      fs.insert(f);
    }

    while (!worklist.empty()) {
      auto* f = worklist.popVal();

      errs() << worklist.size() << "\n";

      auto& fs = fm[f];
      int curSize = fs.size();
      int prevSize = change[f];

      // not changed
      if (curSize == prevSize) {
        continue;
      }

      std::unordered_set<Function*> seen;
      for (auto& I : instructions(*f)) {
        if (auto* cb = dyn_cast<CallBase>(&I)) {
          auto* callee = getCalledFunction(cb);
          
          bool notSeen = !seen.count(callee);
          bool notSkipped = !globals.functions.skipFunction(callee);
          bool notSelf = (callee != f);
          if (notSeen && notSkipped && notSelf) {
            auto& fsCallee = fm[callee];
            fs.extend(fsCallee);
            worklist.insert(callee);
            seen.insert(callee);
          }
        }
      }

      change[f] = fs.size();
      worklist.insert(f);
    }
  }

  void fillAnalyzedFunctions() {
    auto& fm = globals.functions.getAllAnalyzedFunctions();
    for (auto* f : globals.functions.getAnalyzedFunctions()) {
      addAllAnalyzed(f, fm);
    }
    worklistFillAllAnalyzed(fm);
  }

  virtual bool isKnownSkipFunction(Function* f) const {
    (void)(f);
    return false;
  }

  void fillSkipFunctions() {
    for (auto& F : M) {
      if (F.isIntrinsic() || F.isDeclaration())
        continue;

      // insert all skip functions
      auto* lastInstr = Traversal::getFunctionExitKey(&F);
      if (!isa<ReturnInst>(lastInstr) || isKnownSkipFunction(&F))
        globals.functions.insertSkipFunction(&F);
    }
  }

  Module& M;
  Globals& globals;

public:
  FunctionParser(Module& M_, Globals& globals_) : M(M_), globals(globals_) {}

  void parse() {
    fillSkipFunctions();
    addAnnotFuncs();
    addNamedFuncs();
    fillAnalyzedFunctions();
  }
};

} // namespace llvm