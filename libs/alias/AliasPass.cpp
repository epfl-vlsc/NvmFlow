#include "AliasPass.h"
#include "AliasUtils.h"

using namespace std;
namespace llvm {

void AliasPass::print(raw_ostream& OS, const Module* m) const {
  OS << "pass\n";
}

bool isNoAliasCall(const Value* V) {
  if (const auto* Call = dyn_cast<CallBase>(V))
    return Call->hasRetAttr(Attribute::NoAlias);
  return false;
}

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
  

  /*
  auto& TLI = getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();
  AAResults AAR(TLI);
  PointsTo pt(TLI);
  auto& steens = pt.getResult();
  AAR.addAAResult(steens);
  AA a(M, AAR);
  */
/*
  for (auto& F : M) {
    //errs() << F.getName() << "\n";
    for(auto&I : instructions(&F)){
      if(auto* ci = dyn_cast<CallBase>(&I)){
        int retAttr = ci->hasRetAttr(Attribute::NoAlias);
        if(retAttr)
        errs() << I << " " << retAttr << "\n";
      }
    }
  }
*/
  return false;
}

void AliasPass::getAnalysisUsage(AnalysisUsage& AU) const {
  AU.addRequired<TargetLibraryInfoWrapperPass>();
  AU.addRequired<CFLSteensAAWrapperPass>();
  AU.setPreservesAll();
}

char AliasPass::ID = 0;
RegisterPass<AliasPass> X("alias", "Alias pass");

} // namespace llvm