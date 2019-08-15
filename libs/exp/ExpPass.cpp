

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

#include "llvm/ADT/StringSwitch.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/AliasAnalysisEvaluator.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/CFGPrinter.h"
#include "llvm/Analysis/CFLAndersAliasAnalysis.h"
#include "llvm/Analysis/CFLSteensAliasAnalysis.h"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/DemandedBits.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/DominanceFrontier.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/IVUsers.h"
#include "llvm/Analysis/LazyCallGraph.h"
#include "llvm/Analysis/LazyValueInfo.h"
#include "llvm/Analysis/LoopAccessAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/Analysis/ModuleSummaryAnalysis.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Analysis/PhiValues.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/ProfileSummaryInfo.h"
#include "llvm/Analysis/RegionInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionAliasAnalysis.h"
#include "llvm/Analysis/ScopedNoAliasAA.h"
#include "llvm/Analysis/StackSafetyAnalysis.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/TypeBasedAliasAnalysis.h"
#include "llvm/CodeGen/PreISelIntrinsicLowering.h"
#include "llvm/CodeGen/UnreachableBlockElim.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/SafepointIRVerifier.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/Regex.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/AggressiveInstCombine/AggressiveInstCombine.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/IPO/ArgumentPromotion.h"
#include "llvm/Transforms/IPO/CalledValuePropagation.h"
#include "llvm/Transforms/IPO/ConstantMerge.h"
#include "llvm/Transforms/IPO/CrossDSOCFI.h"
#include "llvm/Transforms/IPO/DeadArgumentElimination.h"
#include "llvm/Transforms/IPO/ElimAvailExtern.h"
#include "llvm/Transforms/IPO/ForceFunctionAttrs.h"
#include "llvm/Transforms/IPO/FunctionAttrs.h"
#include "llvm/Transforms/IPO/FunctionImport.h"
#include "llvm/Transforms/IPO/GlobalDCE.h"
#include "llvm/Transforms/IPO/GlobalOpt.h"
#include "llvm/Transforms/IPO/GlobalSplit.h"
#include "llvm/Transforms/IPO/HotColdSplitting.h"
#include "llvm/Transforms/IPO/InferFunctionAttrs.h"
#include "llvm/Transforms/IPO/Inliner.h"
#include "llvm/Transforms/IPO/Internalize.h"
#include "llvm/Transforms/IPO/LowerTypeTests.h"
#include "llvm/Transforms/IPO/PartialInlining.h"
#include "llvm/Transforms/IPO/SCCP.h"
#include "llvm/Transforms/IPO/SampleProfile.h"
#include "llvm/Transforms/IPO/StripDeadPrototypes.h"
#include "llvm/Transforms/IPO/SyntheticCountsPropagation.h"
#include "llvm/Transforms/IPO/WholeProgramDevirt.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Instrumentation.h"
#include "llvm/Transforms/Instrumentation/BoundsChecking.h"
#include "llvm/Transforms/Instrumentation/CGProfile.h"
#include "llvm/Transforms/Instrumentation/ControlHeightReduction.h"
#include "llvm/Transforms/Instrumentation/GCOVProfiler.h"
#include "llvm/Transforms/Instrumentation/InstrProfiling.h"
#include "llvm/Transforms/Instrumentation/MemorySanitizer.h"
#include "llvm/Transforms/Instrumentation/PGOInstrumentation.h"
#include "llvm/Transforms/Instrumentation/ThreadSanitizer.h"
#include "llvm/Transforms/Scalar/ADCE.h"
#include "llvm/Transforms/Scalar/AlignmentFromAssumptions.h"
#include "llvm/Transforms/Scalar/BDCE.h"
#include "llvm/Transforms/Scalar/CallSiteSplitting.h"
#include "llvm/Transforms/Scalar/ConstantHoisting.h"
#include "llvm/Transforms/Scalar/CorrelatedValuePropagation.h"
#include "llvm/Transforms/Scalar/DCE.h"
#include "llvm/Transforms/Scalar/DeadStoreElimination.h"
#include "llvm/Transforms/Scalar/DivRemPairs.h"
#include "llvm/Transforms/Scalar/EarlyCSE.h"
#include "llvm/Transforms/Scalar/Float2Int.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/GuardWidening.h"
#include "llvm/Transforms/Scalar/IVUsersPrinter.h"
#include "llvm/Transforms/Scalar/IndVarSimplify.h"
#include "llvm/Transforms/Scalar/InductiveRangeCheckElimination.h"
#include "llvm/Transforms/Scalar/InstSimplifyPass.h"
#include "llvm/Transforms/Scalar/JumpThreading.h"
#include "llvm/Transforms/Scalar/LICM.h"
#include "llvm/Transforms/Scalar/LoopAccessAnalysisPrinter.h"
#include "llvm/Transforms/Scalar/LoopDataPrefetch.h"
#include "llvm/Transforms/Scalar/LoopDeletion.h"
#include "llvm/Transforms/Scalar/LoopDistribute.h"
#include "llvm/Transforms/Scalar/LoopIdiomRecognize.h"
#include "llvm/Transforms/Scalar/LoopInstSimplify.h"
#include "llvm/Transforms/Scalar/LoopLoadElimination.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"
#include "llvm/Transforms/Scalar/LoopPredication.h"
#include "llvm/Transforms/Scalar/LoopRotation.h"
#include "llvm/Transforms/Scalar/LoopSimplifyCFG.h"
#include "llvm/Transforms/Scalar/LoopSink.h"
#include "llvm/Transforms/Scalar/LoopStrengthReduce.h"
#include "llvm/Transforms/Scalar/LoopUnrollAndJamPass.h"
#include "llvm/Transforms/Scalar/LoopUnrollPass.h"
#include "llvm/Transforms/Scalar/LowerAtomic.h"
#include "llvm/Transforms/Scalar/LowerExpectIntrinsic.h"
#include "llvm/Transforms/Scalar/LowerGuardIntrinsic.h"
#include "llvm/Transforms/Scalar/MakeGuardsExplicit.h"
#include "llvm/Transforms/Scalar/MemCpyOptimizer.h"
#include "llvm/Transforms/Scalar/MergedLoadStoreMotion.h"
#include "llvm/Transforms/Scalar/NaryReassociate.h"
#include "llvm/Transforms/Scalar/NewGVN.h"
#include "llvm/Transforms/Scalar/PartiallyInlineLibCalls.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/RewriteStatepointsForGC.h"
#include "llvm/Transforms/Scalar/SCCP.h"
#include "llvm/Transforms/Scalar/SROA.h"
#include "llvm/Transforms/Scalar/Scalarizer.h"
#include "llvm/Transforms/Scalar/SimpleLoopUnswitch.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Transforms/Scalar/Sink.h"
#include "llvm/Transforms/Scalar/SpeculateAroundPHIs.h"
#include "llvm/Transforms/Scalar/SpeculativeExecution.h"
#include "llvm/Transforms/Scalar/TailRecursionElimination.h"
#include "llvm/Transforms/Scalar/WarnMissedTransforms.h"
#include "llvm/Transforms/Utils/AddDiscriminators.h"
#include "llvm/Transforms/Utils/BreakCriticalEdges.h"
#include "llvm/Transforms/Utils/CanonicalizeAliases.h"
#include "llvm/Transforms/Utils/EntryExitInstrumenter.h"
#include "llvm/Transforms/Utils/LCSSA.h"
#include "llvm/Transforms/Utils/LibCallsShrinkWrap.h"
#include "llvm/Transforms/Utils/LoopSimplify.h"
#include "llvm/Transforms/Utils/LowerInvoke.h"
#include "llvm/Transforms/Utils/Mem2Reg.h"
#include "llvm/Transforms/Utils/NameAnonGlobals.h"
#include "llvm/Transforms/Utils/SymbolRewriter.h"
#include "llvm/Transforms/Vectorize/LoadStoreVectorizer.h"
#include "llvm/Transforms/Vectorize/LoopVectorize.h"
#include "llvm/Transforms/Vectorize/SLPVectorizer.h"

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
  PassBuilder PB;
  FunctionAnalysisManager FA;

  PB.registerFunctionAnalyses(FA);

  FunctionPassManager FPM(true);


  FPM.addPass(SROA());
  //FPM.addPass(EarlyCSEPass(true /* Enable mem-ssa. */));
  
  FPM.addPass(SCCPPass());
  FPM.addPass(ADCEPass());
  FPM.addPass(SimplifyCFGPass());

  for (auto& F : M) {
    if (F.isDeclaration())
      continue;
    FPM.run(F, FA);

    errs() << F.getName() << "\n";
    for (auto& I : instructions(F)) {
      errs() << I << "\n";
    }
  }

  return false;

} // namespace llvm

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