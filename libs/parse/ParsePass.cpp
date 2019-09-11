#include "ParsePass.h"
#include "Common.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/CFLAndersAliasAnalysis.h"
#include "llvm/Analysis/CFLSteensAliasAnalysis.h"

#include "analysis_util/AliasGroups.h"
#include "analysis_util/DfUtil.h"
#include "analysis_util/InstrParser.h"

#include <cassert>
#include <set>
using namespace std;
namespace llvm {

void ParsePass::print(raw_ostream& OS, const Module* m) const { OS << "pass\n"; }

bool ParsePass::runOnModule(Module& M) {


  return false;
}

void ParsePass::getAnalysisUsage(AnalysisUsage& AU) const {
  AU.setPreservesAll();
}

char ParsePass::ID = 0;
RegisterPass<ParsePass> X("parse", "Parse pass");

} // namespace llvm