# useful stuff


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
      auto pv = InstrParser::parseVarLhs(&I);
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
      auto pv = InstrParser::parseVarLhs(&I);
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
      auto pv = InstrParser::parseVarLhs(&I);
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
      auto pv = InstrParser::parseVarLhs(&I);
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
      auto pv = InstrParser::parseVarLhs(&I);
      if (!pv.isUsed())
        continue;

      pv.print(errs());
    }
  }

  errs() << "-------------------------------\n";
}

#$(LL_DIR)/%.ll: $(SRC_DIR)/%.bc
	#llvm-dis -o $@ $<

#
$(LL_DIR)/%.ll: $(SRC_DIR)/%.cpp
	clang++ -S $(INITOPT) $(CPPFLAGS) $(INCFLAGS) -o $@ $<
	opt -S $(CONSFLAGS) -o $@ $@ > /dev/null 2>&1
	clang++ -S $(FINOPT) $(CPPFLAGS) -o $@ $@


tx_begin();  
  TX_ADD(node);
  TX_ADD_FIELD(node, a);
  TX_ADD_FIELD(node, b);
  TX_ADD_FIELD(node, c);
  D_RW(node)->a = 0;
  D_RW(node)->b = 1;
  D_RW(node)->c = TX_ZNEW(struct tree_map_node);
  tx_end();

#include "ExpPass.h"
#include "Common.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/CFLAndersAliasAnalysis.h"
#include "llvm/Analysis/CFLSteensAliasAnalysis.h"
#include "llvm/Analysis/MemoryLocation.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/Analysis/ScalarEvolutionAliasAnalysis.h"
#include "llvm/Analysis/TypeBasedAliasAnalysis.h"
#include "llvm/Transforms/Scalar/SCCP.h"
//#include "llvm/Transforms/IPO/SCCP.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Transforms/Scalar/ADCE.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"

#include "analysis_util/DfUtil.h"

#include "analysis_util/MemoryUtil.h"
#include "llvm/Demangle/Demangle.h"

#include "analysis_util/AliasSets.h"

#include <cassert>
#include <set>
using namespace std;
namespace llvm {

struct PtrFnc {
  Instruction* i;
  Function* f;

  bool isEqual(const PtrFnc& X) const { return i == X.i && f == X.f; }
  bool operator==(const PtrFnc& X) const { return i == X.i && f == X.f; }
  bool operator<(const PtrFnc& X) const { return i < X.i || f < X.f; }
};

template <typename AA> void traverse(Function& F, AA& ast, set<PtrFnc>& p) {
  for (Instruction& I : instructions(F)) {
    Value* ptr = nullptr;
    if (auto* si = dyn_cast<StoreInst>(&I)) {
      ptr = si->getPointerOperand();
    } else if (auto* li = dyn_cast<LoadInst>(&I)) {
      ptr = li->getPointerOperand();
    } else if (auto* ai = dyn_cast<AllocaInst>(&I)) {
      ptr = ai->getOperand(0);
    } else if (auto* ci = dyn_cast<CallInst>(&I)) {
      auto* f = ci->getCalledFunction();
      traverse(*f, ast, p);
    }
    if (ptr && ptr->getType()->isPointerTy()) {
      p.insert(PtrFnc{&I, &F});
    }
  }
}

template <typename AA> void traverse(Function& F, AA& ast) {
  set<PtrFnc> p;
  traverse(F, ast, p);

  /*
  for (auto [i, f] : p)
    errs() << f->getName() << " " << *i << "\n";
*/
  for (auto [i1, f1] : p)
    for (auto [i2, f2] : p) {
      int res = ast.alias(i1, i2);
      if (res && i1 != i2)
        errs() << res << " " << f1->getName() << " " << *i1 << " "
               << f2->getName() << " " << *i2 << "\n";
    }
}

void aliasanalysis() {
  /*
  auto f = M.getFunction("_Z2m3P1A");
    auto f2 = M.getFunction("_Z2m1P1A");
    // auto& mssa = getAnalysis<MemorySSAWrapperPass>(*f).getMSSA();
    // mssa.print(llvm::errs());

    AAResults AAR(getAnalysis<TargetLibraryInfoWrapperPass>().getTLI());
    // auto& aa = getAnalysis<CFLSteensAAWrapperPass>().getResult();
    auto& anders = getAnalysis<CFLAndersAAWrapperPass>().getResult();
    AAR.addAAResult(anders);
    // AliasAnalysis& aa = getAnalysis<AAResultsWrapperPass>(*f).getAAResults();

    AliasSetTracker ast(AAR);
    for (auto& BB : *f) {
      ast.add(BB);
    }
    for (auto& BB : *f2) {
      ast.add(BB);
    }

    ast.print(llvm::errs());

    traverse(*f, AAR);
    */
}

void demangle(Module& M) {
  auto f = M.getFunction("_ZN3Log10correctObjEv");
  char buf[100];
  size_t n;
  int s;
  itaniumDemangle(f->getName().str().c_str(), buf, &n, &s);
  errs() << buf << " " << n << " " << s << "\n";
}

void types() {
  /*for (Instruction& I : instructions(*f)) {
      if (auto* ci = dyn_cast<CallInst>(&I)) {
        auto* f = ci->getCalledFunction();
        if (f->getName() == "_Z3logPv") {
          errs() << "lol\n";
          auto* arg0 = ci->getArgOperand(0);
          auto* uncasted = getUncasted(arg0);
          errs() << *uncasted << "\n";
          auto* type = uncasted->getType();
          assert(type->isPointerTy());
          auto* eType = type->getPointerElementType();
          eType->print(errs());
          if (auto* st = dyn_cast<StructType>(eType)) {
            errs() << "xol\n";
          }
        }
      }
    } */
}

void trav2() {
  /*for (auto& BB : *f) {
    errs() << "bb:" << &BB << "\n";
    for (auto& I : BB) {
      auto* i = &I;
      errs() << "i:" << i << "\n";
    }#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassBuilder.h"
    errs() << "v:" << BB.getTerminator() << "\n";#include
"llvm/Passes/PassBuilder.h" if (auto* ret =
llvm::dyn_cast<llvm::ReturnInst>(BB.getTer#include
"llvm/Passes/PassBuilder.h"minator())) { errs() << "ret:" <<
ret->getReturnValue() << "\n";
    }
  } */
}

void trav3() {
  /*
  auto f = M.getFunction("_Z2m3P1AS0_");
    auto f2 = M.getFunction("_Z2m1P1A");

    AAResults AAR(getAnalysis<TargetLibraryInfoWrapperPass>().getTLI());
    auto& anders = getAnalysis<CFLAndersAAWrapperPass>().getResult();
    AAR.addAAResult(anders);
    AliasSetTracker ast(AAR);
    for (auto& BB : *f) {
      ast.add(BB);
    }
    for (auto& BB : *f2) {
      ast.add(BB);
    }

    ast.print(errs());
  */
}

void ExpPass::print(raw_ostream& OS, const Module* m) const { OS << "pass\n"; }

void morealias() {
  /*
  AAResults AAR(getAnalysis<TargetLibraryInfoWrapperPass>().getTLI());
  // auto& cflResult = getAnalysis<TypeBasedAAWrapperPass>().getResult();
  // auto& cflResult = getAnalysis<SCEVAAWrapperPass>().getResult();
  auto& cflResult = getAnalysis<CFLAndersAAWrapperPass>().getResult();
  // auto& cflResult = getAnalysis<CFLSteensAAWrapperPass>().getResult();
  AAR.addAAResult(cflResult);

  auto* f = M.getFunction("_ZN3Dur7correctEv");

  std::set<Instruction*> ptrs;
  std::map<Instruction*, std::set<Instruction*>> instMap;

  SetVector<Value*> Pointers;
  for (Instruction& I : instructions(*f)) {
    if (I.getType()->isPointerTy()) {
      errs() << I << "\n";
      Pointers.insert(&I);
    }
  }

  for (Value* P1 : Pointers) {
    for (Value* P2 : Pointers) {
      auto res = AAR.alias(P1, P2);
      errs() << res << " " << *P1 << " " << *P2 << " "
             << "\n";
    }
  }
 */

  /*
  AliasSets as(AAR);
  for (Instruction& I : instructions(*f)) {
    if (auto* si = dyn_cast<StoreInst>(&I)) {
      as.add(si);
    } else if (auto* ci = dyn_cast<CallInst>(&I)) {
      auto* f2 = ci->getCalledFunction();
      if (!f2->getName().equals("_Z10clflushoptPKv"))
        continue;

      as.add(ci);
    }
  }
  as.createSets();
  as.print(errs());
  */
}

bool ExpPass::runOnModule(Module& M) {
  for (auto& F : M) {
    errs() << F.getName() << "\n";
    for (auto& I : instructions(F)) {
      errs() << I << "\n";
    }
  }

  return false;
}

void ExpPass::getAnalysisUsage(AnalysisUsage& AU) const {
  // AU.addRequired<TargetLibraryInfoWrapperPass>();
  // AU.addRequired<CFLAndersAAWrapperPass>();

  // AU.addRequired<CFLSteensAAWrapperPass>();
  // AU.addRequired<TypeBasedAAWrapperPass>();
  // AU.addRequired<SCEVAAWrapperPass>();
  // AU.addRequired<AAResultsWrapperPass>();
  // AU.addRequired<MemorySSAWrapperPass>();

  AU.setPreservesAll();
}

char ExpPass::ID = 0;
RegisterPass<ExpPass> X("exp", "Experimental pass");

} // namespace llvm