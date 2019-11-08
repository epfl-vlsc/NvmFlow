#include "ExpPass.h"
#include "Common.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/CFLAndersAliasAnalysis.h"
#include "llvm/Analysis/CFLSteensAliasAnalysis.h"

#include "llvm/Analysis/AliasSetTracker.h"

#include "Locals.h"
#include "analysis_util/Analyzer.h"
#include "data_util/PersistFunctions.h"

#include "analysis_util/DfUtil.h"
#include "parser_util/AliasGroups.h"
#include "parser_util/FunctionParser.h"
#include "parser_util/InstrParser.h"

#include "data_util/CheckReachable.h"

#include <cassert>
#include <set>
using namespace std;
namespace llvm {

void ExpPass::print(raw_ostream& OS, const Module* m) const { OS << "pass\n"; }

Function* addAnnotFuncs(Module& M) {
  static constexpr const char* GLOBAL_ANNOT = "llvm.global.annotations";
  for (Module::global_iterator I = M.global_begin(), E = M.global_end(); I != E;
       ++I) {
    if (I->getName() == GLOBAL_ANNOT) {
      ConstantArray* CA = dyn_cast<ConstantArray>(I->getOperand(0));
      for (auto OI = CA->op_begin(); OI != CA->op_end(); ++OI) {
        ConstantStruct* CS = dyn_cast<ConstantStruct>(OI->get());
        /*
        GlobalVariable* AnnotationGL =
            dyn_cast<GlobalVariable>(CS->getOperand(1)->getOperand(0));
        StringRef annotation =
            dyn_cast<ConstantDataArray>(AnnotationGL->getInitializer())
                ->getAsCString();
        */

        Function* annotatedFunction =
            dyn_cast<Function>(CS->getOperand(0)->getOperand(0));

        return annotatedFunction;
      }
    }
  }
  return nullptr;
}

/*
auto* f = addAnnotFuncs(M);
for (auto& I : instructions(*f)) {
  if (auto* ci = dyn_cast<CallBase>(&I)) {
    auto* f2 = ci->getCalledFunction();
    if (!f2) {
      errs() << DbgInstr::getSourceLocation(ci) << " " << I << "\n";
      Value* v = ci->getCalledValue();
      auto* calledFunction = dyn_cast<Function>(v->stripPointerCasts());
      errs() << *calledFunction << "\n";
    }
  }
}

*/

void trav(Module& M, std::set<Instruction*>& is) {
  for (auto& F : M) {
    for (auto& I : instructions(F)) {
      is.insert(&I);
    }
  }
}

bool ExpPass::runOnModule(Module& M) {
  using Globals = GlobalStore<PersistFunctions, Locals>;
  AAResults AAR(getAnalysis<TargetLibraryInfoWrapperPass>().getTLI());
  Globals globals(M, AAR);

  FunctionParser fp(M, globals);
  fp.parse();

  CheckReachable CR(globals);

  globals.functions.printAllAnalyzed(errs());

  for (auto& F1 : M) {
    for (auto& F2 : M) {
      auto r = CR.fncCallsFnc(&F1, &F2);
      errs() << (int)r << " " << F1.getName() << " " << F2.getName() << "\n";
    }
  }

  /*

  std::set<Instruction*> is;
  trav(M, is);

  for (auto* i1 : is) {
    for (auto* i2 : is) {
      bool r = CR.instReachesInst(i1, i2);
      errs() << (int)r << " " << DbgInstr::getSourceLocation(i1) << " "
             << DbgInstr::getSourceLocation(i2) << "\n";
    }
  }
  */

  return false;
}

void ExpPass::getAnalysisUsage(AnalysisUsage& AU) const {
  AU.addRequired<TargetLibraryInfoWrapperPass>();
  AU.setPreservesAll();
}

char ExpPass::ID = 0;
RegisterPass<ExpPass> X("exp", "Exp pass");

} // namespace llvm