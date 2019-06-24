

#include "LogPass.h"

namespace llvm {

void LogPass::print(raw_ostream& OS, const Module* m) const {
  OS << "print bugs here\n";
}

bool LogPass::runOnModule(Module& M) {

  LogAnalyzer analyzer(M);

  analyzer.parse();

  analyzer.dataflow();

  analyzer.report();
  return false;
}

void LogPass::getAnalysisUsage(AnalysisUsage& AU) const {
}

char LogPass::ID = 0;
RegisterPass<LogPass> X("log", "Log pass");

} // namespace llvm