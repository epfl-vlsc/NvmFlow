#include "ParsePass.h"
#include "Common.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/CFLAndersAliasAnalysis.h"
#include "llvm/Analysis/CFLSteensAliasAnalysis.h"

#include "analysis_util/AliasGroups.h"
#include "analysis_util/DfUtil.h"
#include "parser_util/InstrParser.h"

#include <algorithm>
#include <cassert>
#include <set>
using namespace std;
namespace llvm {

void ParsePass::print(raw_ostream& OS, const Module* m) const {
  OS << "pass\n";
}

bool isSkipFunction(Function& F) {
  static const char* skipFunctions[] = {"_ZL9TX_MEMCPYPvPKvm",
                                        "_ZL9TX_MEMSETPvim"};
  for (auto* c : skipFunctions) {
    if (F.getName().equals(c))
      return true;
  }

  return false;
}

bool takeFunction(Function& F) {
  static const char* takeFunctions[] = {//"_Z3rhsP4tree",
                                        "_Z6alllhsP4tree",
                                        //"_Z4dptrPP4node",
                                        //"_Z4simpP4tree"
                                        };
  for (auto* c : takeFunctions) {
    if (F.getName().equals(c))
      return true;
  }

  return false;
}

bool ParsePass::runOnModule(Module& M) {
  for (auto& F : M) {
    if (F.isIntrinsic() || F.isDeclaration() || isSkipFunction(F))
      continue;

    if (!takeFunction(F))
      continue;

    errs() << "function:" << F.getName() << "\n";
    for (auto& I : instructions(F)) {
      auto pv = InstrParser::parseInstruction(&I);

      if (pv.isUsed()) {
        pv.print(errs());
        errs() << I << "\n";
      }
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