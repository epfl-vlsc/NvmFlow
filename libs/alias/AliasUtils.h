#pragma once
#include "Common.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/CFLAndersAliasAnalysis.h"
#include "llvm/Analysis/CFLSteensAliasAnalysis.h"

#include "llvm/Analysis/AliasSetTracker.h"

#include "analysis_util/DfUtil.h"
#include "parser_util/AliasGroups.h"
#include "parser_util/InstrParser.h"

#include <cassert>
#include <set>
using namespace std;
namespace llvm {

struct AA {
  std::set<Value*> values;
  SparseAliasGroups ag;

  AA(Module& M, AAResults& AAR) : ag(AAR) {
    auto& F = *M.getFunction("main");
    traverse(F);
    analyze(AAR);
    ag.print(errs());
  }

  void traverse(Function& F) {
    errs() << F.getName() << "\n";
    for (auto& I : instructions(F)) {
      auto pv = InstrParser::parseVarLhs(&I);
      if (pv.isUsed()) {
        pv.print(errs());
        auto* obj = pv.getObj();
        //auto* opnd = pv.getOpnd();

        values.insert(obj);
        //values.insert(opnd);

        //ag.insert(opnd);
        ag.insert(obj);
      } else if (auto* ci = dyn_cast<CallInst>(&I)) {
        auto* f = getCalledFunction(ci);
        if (f->isDeclaration() || f->isIntrinsic())
          continue;

        traverse(*f);
      }
    }
  }

  void analyze(AAResults& AAR) {
    errs() << "Analyze\n";
    for (Value* v1 : values)
      for (Value* v2 : values) {
        auto a1 = AAR.alias(v1, v2);
        auto a2 = AAR.alias(v2, v1);
        assert(a1 == a2);
        errs() << a1 << " " << *v1 << " " << *v2 << "\n";
      }
  }
};

} // namespace llvm
