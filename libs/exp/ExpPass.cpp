

#include "ExpPass.h"
#include "Common.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/CFLAndersAliasAnalysis.h"
#include "llvm/Analysis/CFLSteensAliasAnalysis.h"
#include "llvm/Analysis/MemoryLocation.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/Analysis/ScalarEvolutionAliasAnalysis.h"
#include "llvm/Analysis/TypeBasedAliasAnalysis.h"
#include "llvm/Passes/PassBuilder.h"

#include "analysis_util/DfUtil.h"

#include "parser_util/InstrParser.h"

#include "parser_util/AliasGroups.h"
//#include "llvm/Analysis/AliasSetTracker.h"

#include <cassert>
#include <set>
using namespace std;
namespace llvm {

void ExpPass::print(raw_ostream& OS, const Module* m) const { OS << "pass\n"; }

bool ExpPass::runOnModule(Module& M) {
  /*
  AAResults AAR(getAnalysis<TargetLibraryInfoWrapperPass>().getTLI());
  auto& cflResult = getAnalysis<CFLAndersAAWrapperPass>().getResult();
  AAR.addAAResult(cflResult);
  auto* f = M.getFunction("_ZN3Dur7correctEv");

  AliasGroups ags;
  for (Instruction& I : instructions(*f)) {
    if (auto* si = dyn_cast<StoreInst>(&I)) {
      ags.add(si);
    } else if (auto* ci = dyn_cast<CallInst>(&I)) {
      auto* f2 = ci->getCalledFunction();
      if (!f2->getName().equals("_Z10clflushoptPKv"))
        continue;

      ags.add(ci);
    }
  }
  ags.createGroups(&AAR);
  ags.print(errs());
   */

  for (auto& F : M) {
    if (F.getName().contains("m1"))
      for (Instruction& I : instructions(F)) {
        if (auto* ci = dyn_cast<CallInst>(&I)) {
          auto* f = ci->getCalledFunction();
          if (f->isIntrinsic() || f->getName().contains("_Znwm") ||
              f->getName().contains("x"))
            continue;
        }

        auto pv = InstrParser::parseInstruction(&I);
        errs() << I << "\n";
        pv.print(errs());
        errs() << "\n";
      }
  }

  return false;
}

void ExpPass::getAnalysisUsage(AnalysisUsage& AU) const {
  AU.addRequired<TargetLibraryInfoWrapperPass>();
  AU.addRequired<CFLAndersAAWrapperPass>();

  // AU.addRequired<CFLSteensAAWrapperPass>();
  // AU.addRequired<TypeBasedAAWrapperPass>();
  // AU.addRequired<SCEVAAWrapperPass>();
  // AU.addRequired<AAResultsWrapperPass>();
  // AU.addRequired<MemorySSAWrapperPass>();

  AU.setPreservesAll();
}

char ExpPass::ID = 0;
RegisterPass<ExpPass> X("exp", "Experimental pass");

} // namespace llvm