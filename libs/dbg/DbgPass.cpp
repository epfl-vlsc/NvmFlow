

#include "DbgPass.h"
#include "Common.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/CFLAndersAliasAnalysis.h"
#include "llvm/Analysis/CFLSteensAliasAnalysis.h"

#include "analysis_util/AliasGroups.h"
#include "analysis_util/DfUtil.h"
#include "parser_util/InstrParser.h"

#include <cassert>
#include <set>
using namespace std;
namespace llvm {

void DbgPass::print(raw_ostream& OS, const Module* m) const { OS << "pass\n"; }

bool DbgPass::runOnModule(Module& M) {
  DbgInfo dbgInfo(M);
  dbgInfo.print(errs());

  return false;
}

void DbgPass::getAnalysisUsage(AnalysisUsage& AU) const {

  AU.setPreservesAll();
}

char DbgPass::ID = 0;
RegisterPass<DbgPass> X("dbg", "Dbg pass");

} // namespace llvm