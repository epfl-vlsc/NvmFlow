#include "AliasPass.h"
#include "Common.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/CFLAndersAliasAnalysis.h"
#include "llvm/Analysis/CFLSteensAliasAnalysis.h"

#include "llvm/Analysis/AliasSetTracker.h"

#include "analysis_util/AliasGroups.h"
#include "analysis_util/DfUtil.h"
#include "parser_util/InstrParser.h"

#include <cassert>
#include <set>
using namespace std;
namespace llvm {

void AliasPass::print(raw_ostream& OS, const Module* m) const {
  OS << "pass\n";
}

/*
std::set<Value*> vals;
  for (auto& I : instructions(F)) {
    if (auto* si = dyn_cast<StoreInst>(&I)) {
      auto* ptrOpnd = si->getPointerOperand();
      auto* valOpnd = si->getValueOperand();

      auto* ptrType = ptrOpnd->getType();
      auto* valType = valOpnd->getType();

      errs() << *si << "\n";
      errs() << "\t" << *ptrOpnd << " ";
      ptrType->print(errs());
      errs() << "\n";

      errs() << "\t" << *valOpnd << " ";
      valType->print(errs());
      errs() << "\n";

      if (ptrType->isPointerTy())
        vals.insert(ptrOpnd);

      if (valType->isPointerTy())
        vals.insert(valOpnd);

      valType->print(errs());
    } else if (auto* ci = dyn_cast<CallInst>(&I)) {
      if (isa<DbgValueInst>(ci))
        continue;

      auto* ptrOpnd = ci->getArgOperand(0);

      auto* ptrType = ptrOpnd->getType();

      errs() << *ci << "\n";
      errs() << "\t" << *ptrOpnd << " ";
      ptrType->print(errs());
      errs() << "\n";

      if (ptrType->isPointerTy())
        vals.insert(ptrOpnd);

      ptrType->print(errs());
    } else if (auto* li = dyn_cast<LoadInst>(&I)) {
      auto* ptrOpnd = li->getPointerOperand();

      auto* ptrType = ptrOpnd->getType();

      errs() << *li << "\n";
      errs() << "\t" << *ptrOpnd << " ";
      ptrType->print(errs());
      errs() << "\n";

      if (ptrType->isPointerTy())
        vals.insert(ptrOpnd);
    }
  }
  for (auto& v1 : vals) {
    for (auto& v2 : vals) {
      auto res = AAR.alias(v1, v2);
      errs() << res << " " << *v1 << " " << *v2 << "\n";
    }
  }
*/

void runAliasAnders(Module& M, CFLAndersAAResult& CFLAAR) {
  errs() << "anders group-------------------\n";
  std::set<Value*> values;

  for (auto& F : M) {
    for (auto& I : instructions(F)) {
      auto pv = InstrParser::parseInstruction(&I);
      if (!pv.isUsed())
        continue;
    }
  }

  std::set<Value*> seen;
  for (auto* v1 : values) {
    for (auto* v2 : values) {
      if (seen.count(v2))
        continue;

      auto* i1 = dyn_cast<Instruction>(v1);
      auto* i2 = dyn_cast<Instruction>(v2);
      if (!i1 || !i2)
        continue;

      auto m1 = MemoryLocation(v1, 8);
      auto m2 = MemoryLocation(v2, 8);
      auto res = CFLAAR.alias(m1, m2);
      errs() << res << " " << DbgInstr::getSourceLocation(i1) << " "
             << DbgInstr::getSourceLocation(i2) << "\n";
    }
    seen.insert(v1);
  }
  errs() << "-------------------------------\n";
}

void runAliasGroup(Module& M, AAResults& AAR) {
  errs() << "alias group--------------------\n";
  AliasGroups ag(AAR);

  for (auto& F : M) {
    for (auto& I : instructions(F)) {
      auto pv = InstrParser::parseInstruction(&I);
      if (!pv.isUsed())
        continue;
    }
  }

  ag.print(errs());
  errs() << "-------------------------------\n";
}

void runAliasSetTracker(Module& M, AAResults& AAR) {
  errs() << "alias set tracker--------------\n";
  AliasSetTracker ast(AAR);

  int added = 0;
  int total = 0;
  for (auto& F : M) {
    for (auto& I : instructions(F)) {
      auto pv = InstrParser::parseInstruction(&I);
      if (!pv.isUsed())
        continue;

      // memcpy
      if (auto* ci = dyn_cast<CallInst>(&I)) {
        total++;

        auto* v = ci->getOperand(0);
        if (auto* i = dyn_cast<Instruction>(v)) {
          ast.add(i);
          added++;
        } else {
          report_fatal_error("cry1");
        }
      } else if (auto* si = dyn_cast<StoreInst>(&I)) {
        total++;

        auto* v = si->getPointerOperand();
        if (auto* i = dyn_cast<Instruction>(v)) {
          ast.add(i);
          added++;
        } else {
          report_fatal_error("cry2");
        }

        v = si->getValueOperand();
        auto* type = v->getType();
        if (!type->isPointerTy())
          continue;

        total++;

        if (auto* i = dyn_cast<Instruction>(v)) {
          ast.add(i);
          added++;
        } else {
          // report_fatal_error("cry3");
          pv.print(errs());
          errs() << "cry3: " << *v << "\n";
        }
      } else {
        report_fatal_error("farcry");
      }
    }
  }
  errs() << "total " << total << " added " << added << "\n";
  ast.print(errs());
  errs() << "-------------------------------\n";
}

void runAlias(Module& M, AAResults& AAR) {
  errs() << "anders group-------------------\n";
  std::set<Value*> values;

  for (auto& F : M) {
    for (auto& I : instructions(F)) {
      auto pv = InstrParser::parseInstruction(&I);
      if (!pv.isUsed())
        continue;
      
    }
  }

  std::set<Value*> seen;
  for (auto* v1 : values) {
    for (auto* v2 : values) {
      if (seen.count(v2))
        continue;

      auto res = AAR.alias(v1, v2);
      errs() << res << " " << *v1 << " " << *v2 << "\n";
    }
    seen.insert(v1);
  }
  errs() << "-------------------------------\n";
}

void printPV(Module& M) {
  errs() << "pv-----------------------------\n";

  for (auto& F : M) {
    for (auto& I : instructions(F)) {
      auto pv = InstrParser::parseInstruction(&I);
      if (!pv.isUsed())
        continue;

      pv.print(errs());
    }
  }

  errs() << "-------------------------------\n";
}

bool AliasPass::runOnModule(Module& M) {
  AAResults AAR(getAnalysis<TargetLibraryInfoWrapperPass>().getTLI());
  // auto& cflResult = getAnalysis<CFLAndersAAWrapperPass>().getResult();
  auto& cflResult = getAnalysis<CFLSteensAAWrapperPass>().getResult();
  AAR.addAAResult(cflResult);

  printPV(M);
  //runAlias(M, AAR);
  // runAliasGroup(M, AAR);
  // runAliasSetTracker(M, AAR);
  // runAliasAnders(M, cflResult);

  return false;
}

void AliasPass::getAnalysisUsage(AnalysisUsage& AU) const {
  AU.addRequired<TargetLibraryInfoWrapperPass>();
  AU.addRequired<CFLAndersAAWrapperPass>();
  AU.addRequired<CFLSteensAAWrapperPass>();

  AU.setPreservesAll();
}

char AliasPass::ID = 0;
RegisterPass<AliasPass> X("alias", "Alias pass");

} // namespace llvm