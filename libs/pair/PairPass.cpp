#include "PairPass.h"

#include "analysis_util/Analyzer.h"
#include "ds/Functions.h"
#include "ds/Locals.h"
#include "ds/Variable.h"
#include "preprocess/VariableParser.h"

#include "state/BugReporter.h"
#include "state/Lattice.h"
#include "state/Transfer.h"

namespace llvm {

void PairPass::print(raw_ostream& OS, const Module* m) const {
  OS << "print bugs here\n";
}

bool PairPass::runOnModule(Module& M) {
  auto& TLI = getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();
  AAResults AAR(TLI);

  // boost analysis with andersen analysis
  auto& aaResults = getAnalysis<CFLAndersAAWrapperPass>().getResult();
  AAR.addAAResult(aaResults);

  using Globals = ProgramStore<Functions, Locals>;
  using VarParser = VariableParser<Globals>;

  using LatVar = Variable*;
  using LatVal = Lattice;
  using State = std::map<LatVar, LatVal>;
  using Transition = Transfer<Globals, LatVar, LatVal>;
  using BReporter = BugReporter<Globals, LatVar, LatVal>;

  using PairAnalyzer =
      Analyzer<Globals, VarParser, State, Transition, BReporter>;
  PairAnalyzer analyzer(M, AAR);
  return false;
}

void PairPass::getAnalysisUsage(AnalysisUsage& AU) const {
  AU.addRequired<TargetLibraryInfoWrapperPass>();
  AU.addRequired<CFLAndersAAWrapperPass>();

  AU.setPreservesAll();
}

char PairPass::ID = 0;
RegisterPass<PairPass> X("pair", "Pair pass");

} // namespace llvm