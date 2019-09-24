#include "LogPass.h"

#include "analysis_util/Analyzer.h"
#include "ds/Locals.h"
#include "ds/Variable.h"
#include "ds/Functions.h"
#include "preprocess/VariableParser.h"
#include "preprocess/FuncParser.h"
#include "parser_util/Parser.h"

#include "state/BugReporter.h"
#include "state/Lattice.h"
#include "state/Transfer.h"

namespace llvm {

void LogPass::print(raw_ostream& OS, const Module* m) const {
  OS << "print bugs here\n";
}

bool LogPass::runOnModule(Module& M) {
  auto& TLI = getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();
  AAResults AAR(TLI);

  // boost analysis with andersen analysis
  auto& aaResults = getAnalysis<CFLAndersAAWrapperPass>().getResult();
  AAR.addAAResult(aaResults);

  using Globals = GlobalStore<Functions, Locals>;
  using FuncParser = FuncParser<Globals>;
  using VarParser = VariableParser<Globals>;
  using AllParser = Parser<Globals, FuncParser, VarParser>;

  using LatVar = Variable*;
  using LatVal = Lattice;
  using State = std::map<LatVar, LatVal>;
  using BReporter = BugReporter<Globals>;
  using Transition = Transfer<Globals, BReporter>;

  using LogAnalyzer =
      Analyzer<Globals, AllParser, State, Transition, BReporter>;
  LogAnalyzer analyzer(M, AAR);
  return false;
}

void LogPass::getAnalysisUsage(AnalysisUsage& AU) const {
  AU.addRequired<TargetLibraryInfoWrapperPass>();
  AU.addRequired<CFLAndersAAWrapperPass>();

  AU.setPreservesAll();
}

char LogPass::ID = 0;
RegisterPass<LogPass> X("log", "Log pass");

} // namespace llvm