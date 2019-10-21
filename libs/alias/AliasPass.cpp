#include "AliasPass.h"
#include "Common.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/CFLAndersAliasAnalysis.h"
#include "llvm/Analysis/CFLSteensAliasAnalysis.h"

#include "llvm/Analysis/AliasSetTracker.h"

#include "parser_util/AliasGroups.h"
#include "analysis_util/DfUtil.h"
#include "parser_util/InstrParser.h"

#include <cassert>
#include <set>
using namespace std;
namespace llvm {

void AliasPass::print(raw_ostream& OS, const Module* m) const {
  OS << "pass\n";
}

struct AA {
  std::set<Value*> values;
  AliasGroups ag;

  AA(Module& M, AAResults& AAR) : ag(AAR) {
    auto& F = *M.getFunction("main");
    traverse(F);
    analyze(AAR);
    ag.print(errs());
  }

  void traverse(Function& F) {
    errs() << F.getName() << "\n";
    for (auto& I : instructions(F)) {
      auto pv = InstrParser::parseVarLhs(&I);
      if (pv.isUsed()) {
        pv.print(errs());
        auto* obj = pv.getObj();
        auto* opnd = pv.getOpnd();

        values.insert(obj);
        values.insert(opnd);

        ag.insert(opnd);
        ag.insert(obj);
      } else if (auto* ci = dyn_cast<CallInst>(&I)) {
        auto* f = ci->getCalledFunction();
        if (f->isDeclaration() || f->isIntrinsic())
          continue;

        traverse(*f);
      }
    }
  }

  void analyze(AAResults& AAR) {
    errs() << "Analyze\n";
    for (Value* v1 : values)
      for (Value* v2 : values)
        errs() << AAR.alias(v1, v2) << " " << *v1 << " " << *v2 << "\n";
  }
};

bool AliasPass::runOnModule(Module& M) {
  AAResults AAR(getAnalysis<TargetLibraryInfoWrapperPass>().getTLI());
  // auto& cflResult = getAnalysis<CFLAndersAAWrapperPass>().getResult();
  auto& cflResult = getAnalysis<CFLSteensAAWrapperPass>().getResult();
  AAR.addAAResult(cflResult);

  AA a(M, AAR);
  // printPV(M);
  // runAlias(M, AAR);
  // runAliasGroup(M, AAR);
  // runAliasSetTracker(M, AAR);
  // runAliasAnders(M, cflResult);

  return false;
}

void AliasPass::getAnalysisUsage(AnalysisUsage& AU) const {
  AU.addRequired<TargetLibraryInfoWrapperPass>();
  AU.addRequired<CFLAndersAAWrapperPass>();
  AU.addRequired<CFLSteensAAWrapperPass>();

  AU.setPreservesAll();
}

char AliasPass::ID = 0;
RegisterPass<AliasPass> X("alias", "Alias pass");

} // namespace llvm