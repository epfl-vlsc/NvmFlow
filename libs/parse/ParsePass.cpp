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
  static const char* takeFunctions[] = {
      //"_Z3rhsP4tree",
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

bool analyzeFunction(CallInst* ci) {
  Function& F = *ci->getCalledFunction();
  static const char* analyzeFunctions[] = {"llvm.memcpy.p0i8.p0i8.i64",
                                           "_Z13pm_flushfencePKv"};
  for (auto* c : analyzeFunctions) {
    if (F.getName().equals(c))
      return true;
  }

  return false;
}

Value* backtrace(Value* v) {
  bool cnt = true;
  while (cnt) {
    if (auto* si = dyn_cast<StoreInst>(v)) {
      v = si->getPointerOperand();
    } else if (auto* ci = dyn_cast<CastInst>(v)) {
      v = ci->getOperand(0);
    } else if (auto* gepi = dyn_cast<GetElementPtrInst>(v)) {
      v = gepi->getPointerOperand();
    } else if (auto* li = dyn_cast<LoadInst>(v)) {
      v = li->getPointerOperand();
    } else if (auto* ii = dyn_cast<IntrinsicInst>(v)) {
      v = ii->getOperand(0);
    } else if (auto* cii = dyn_cast<CallInst>(v)) {
      v = cii->getOperand(0);
    } else if (auto* ai = dyn_cast<AllocaInst>(v)) {
      cnt = false;
    } else if (auto* bo = dyn_cast<BinaryOperator>(v)) {
      v = bo->getOperand(0);
    } else if (auto* pi = dyn_cast<PtrToIntInst>(v)) {
      v = pi->getPointerOperand();
    } else if (auto* a = dyn_cast<Argument>(v)) {
      cnt = false;
    } else if (auto* c = dyn_cast<Constant>(v)) {
      cnt = false;
    } else if (auto* iii = dyn_cast<InvokeInst>(v)) {
      cnt = false;
    } else {
      cnt = false;
    }

    errs() << "\t" << *v << "\n";
  }

  return v;
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
      if (!pv.isUsed())
        continue;

      pv.print(errs());
      errs() << I << "\n";
      
      Value* v = nullptr;

      if (auto* si = dyn_cast<StoreInst>(&I)) {
        v = si->getPointerOperand();
      } else if (auto* ci = dyn_cast<CallInst>(&I)) {
        if (!analyzeFunction(ci))
          continue;

        v = ci->getOperand(0);
      }
      if (v){
        backtrace(v);
        errs() << "\n\n";
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