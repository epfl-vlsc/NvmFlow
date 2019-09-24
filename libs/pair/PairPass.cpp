#include "PairPass.h"

#include "analysis_util/Analyzer.h"
#include "data_util/PersistFunctions.h"
#include "ds/Locals.h"
#include "ds/Variable.h"
#include "preprocess/VariableParser.h"
#include "parser_util/FunctionParser.h"
#include "parser_util/Parser.h"

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

  using Globals = GlobalStore<PersistFunctions, Locals>;
  using FuncParser = FunctionParser<Globals>;
  using VarParser = VariableParser<Globals>;
  using AllParser = Parser<Globals, FuncParser, VarParser>;

  using LatVar = Variable*;
  using LatVal = Lattice;
  using State = std::map<LatVar, LatVal>;
  using BReporter = BugReporter<Globals>;
  using Transition = Transfer<Globals, BReporter>;

  using PairAnalyzer =
      Analyzer<Globals, AllParser, State, Transition, BReporter>;
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