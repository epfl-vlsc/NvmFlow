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

void ParsePass::print(raw_ostream& OS, const Module* m) const {
  OS << "pass\n";
}

bool ParsePass::runOnModule(Module& M) {
  for (auto& F : M) {
    if (F.isIntrinsic() || F.isDeclaration())
      continue;

    errs() << F.getName() << "\n";
    for (auto& I : instructions(F)) {
      auto pv = InstrParser::parseInstruction(&I);
      pv.print(errs());
    }
  }

  return false;
}

void ParsePass::getAnalysisUsage(AnalysisUsage& AU) const {
  AU.setPreservesAll();
}

char ParsePass::ID = 0;
RegisterPass<ParsePass> X("parse", "Parse pass");

} // namespace llvm