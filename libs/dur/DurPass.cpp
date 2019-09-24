#include "DurPass.h"

#include "analysis_util/Analyzer.h"
#include "checker_util/Functions.h"
#include "ds/Locals.h"
#include "ds/Variable.h"
#include "preprocess/VariableParser.h"

#include "state/BugReporter.h"
#include "state/Lattice.h"
#include "state/Transfer.h"

namespace llvm {

void DurPass::print(raw_ostream& OS, const Module* m) const {
  OS << "print bugs here\n";
}

bool DurPass::runOnModule(Module& M) {
  // initialize alias analysis
  auto& TLI = getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();
  AAResults AAR(TLI);

  // boost analysis with andersen analysis
  auto& aaResults = getAnalysis<CFLAndersAAWrapperPass>().getResult();
  AAR.addAAResult(aaResults);

  using Globals = GlobalStore<Functions, Locals>;
  using VarParser = VariableParser<Globals>;

  using LatVar = Variable*;
  using LatVal = Lattice;
  using State = std::map<LatVar, LatVal>;
  using BReporter = BugReporter<Globals>;
  using Transition = Transfer<Globals, BReporter>;

  using DurAnalyzer =
      Analyzer<Globals, VarParser, State, Transition, BReporter>;
  DurAnalyzer analyzer(M, AAR);
  return false;
}

void DurPass::getAnalysisUsage(AnalysisUsage& AU) const {
  AU.addRequired<TargetLibraryInfoWrapperPass>();
  AU.addRequired<CFLAndersAAWrapperPass>();

  AU.setPreservesAll();
}

char DurPass::ID = 0;
RegisterPass<DurPass> X("dur", "Durable pointer pass");

} // namespace llvm