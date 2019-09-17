#pragma once
#include "Common.h"

#include "ds/InstrInfo.h"
#include "parser_util/InstrParser.h"
#include "llvm/ADT/EquivalenceClasses.h"

namespace llvm {

template <typename Globals> class AliasParser {
  using InstrType = typename InstrInfo::InstrType;

  static constexpr const char* DurableField = "DurableField";

  struct LocRefData {
    Instruction* instr;
    InstrType instrType;
    Variable var;
    Value* rhsAlias;
  };

  auto getCallInstrType(CallInst* ci) const {
    auto* callee = ci->getCalledFunction();

    if (globals.functions.isSkippedFunction(callee)) {
      return InstrType::None;
    } else if (globals.functions.isPfenceFunction(callee)) {
      return InstrType::PfenceInstr;
    } else if (globals.functions.isVfenceFunction(callee)) {
      return InstrType::VfenceInstr;
    } else if (globals.functions.isFlushFunction(callee)) {
      return InstrType::FlushInstr;
    } else if (globals.functions.isFlushFenceFunction(callee)) {
      return InstrType::FlushFenceInstr;
    } else {
      return InstrType::IpInstr;
    }
  }

  auto getInstrType(Instruction* i) const {
    if (auto* si = dyn_cast<StoreInst>(i)) {
      return InstrType::WriteInstr;
    } else if (auto* ci = dyn_cast<CallInst>(i)) {
      return getCallInstrType(ci);
    }

    return InstrType::None;
  }

  void addVarReferences(Function* func) {
    for (auto* f : globals.functions.getUnitFunctions(func)) {
      for (auto& I : instructions(*f)) {
        // get instruction type
        auto instrType = getInstrType(&I);
        if (!InstrInfo::isUsedInstr(instrType))
          continue;

        // check non variable based parsing
        if (InstrInfo::isNonVarInstr(instrType)) {
          globals.locals.addInstrInfo(&I, instrType, nullptr, nullptr);
          continue;
        }

        // parse variable based
        auto pv = InstrParser::parseInstruction(&I);
        if (!pv.isUsed())
          continue;

        // check tracked types
        auto* type = pv.getType();
        if (!globals.dbgInfo.isTrackedType(type))
          continue;

        // variable info
        StructField* sf = nullptr;
        bool annotated =
            pv.isAnnotated() && pv.getAnnotation().equals(DurableField);

        if (pv.isField()) {
          auto [st, idx] = pv.getStructInfo();

          if (!globals.dbgInfo.isUsedStructType(st))
            continue;

          sf = globals.dbgInfo.getStructField(st, idx);
        }

        // find local name
        auto* lv = pv.getLocalVar();
        // todo need to handle phi
        auto* diVar = globals.dbgInfo.getDILocalVariable(lv);
        std::string localName = (diVar) ? diVar->getName().str() : "";

        auto var = Variable::getVariable(pv, sf, annotated, localName);
        auto* rhs = pv.getRhs();

        // only var references in this loop
        if (pv.isLocRef()) {
          // location references
          locReferences.push_back({&I, instrType, var, rhs});
        } else {
          // var references
          auto* varPtr = globals.locals.addVariable(var);
          globals.locals.addInstrInfo(&I, instrType, varPtr, rhs);
        }
      }
    }
  }

  void addLocReferences() {}

  void addLocals() {
    for (auto* f : globals.functions.getAnalyzedFunctions()) {
      globals.setActiveFunction(f);
      addVarReferences(f);
      addLocReferences();
    }
  }

  Globals& globals;
  std::vector<LocRefData> locReferences;

public:
  AliasParser(Globals& globals_) : globals(globals_) { addLocals(); }
};

} // namespace llvm