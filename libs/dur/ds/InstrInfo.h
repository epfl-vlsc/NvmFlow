#pragma once
#include "Common.h"
#include "Variable.h"
#include "parser_util/InstrParser.h"

namespace llvm {

struct InstrInfo {
  enum InstrType {
    WriteInstr,
    FlushInstr,
    FlushFenceInstr,
    VfenceInstr,
    PfenceInstr,
    IpInstr,
    None
  };

  static constexpr const char* Strs[] = {
      "write", "flush", "flushfence", "vfence", "pfence", "ip", "none"};
  Instruction* instr;
  InstrType instrType;

  Variable* aliasLhs;
  Variable* aliasRhs;

  ParsedVariable pvLhs;
  ParsedVariable pvRhs;

  InstrInfo() : instrType(None) {}

  InstrInfo(Instruction* instr_, InstrType instrType_, Variable* aliasLhs_,
            Variable* aliasRhs_, ParsedVariable& pvLhs_, ParsedVariable& pvRhs_)
      : instr(instr_), instrType(instrType_), aliasLhs(aliasLhs_),
        aliasRhs(aliasRhs_), pvLhs(pvLhs_), pvRhs(pvRhs_) {
    assert(instr);
    assert(instrType != None);
  }

  auto getInstrType() const {
    assert(instr);
    return instrType;
  }

  bool isIpInstr() const {
    assert(instr);
    return instrType == IpInstr;
  }

  auto* getVariableLhs() {
    assert(aliasLhs);
    return aliasLhs;
  }

  bool hasVariableRhs() const { return pvRhs.isUsed() && aliasRhs; }

  auto* getVariableRhs() {
    assert(aliasRhs);
    return aliasRhs;
  }

  auto* getInstruction() {
    assert(instr);
    return instr;
  }

  void print(raw_ostream& O) const {
    assert(instr);
    O << Strs[(int)instrType];

    auto sl = DbgInstr::getSourceLocation(instr);
    if (!sl.empty())
      O << " " + sl;

    O << "\t";
    pvLhs.print(O, false);
    O << "\t";
    pvRhs.print(O, false);
    O << "\n";
  }

  bool isUsedInstr() const { return instrType != None; }

  bool isFlushBasedInstr() const {
    return instrType == FlushInstr || instrType == FlushFenceInstr;
  }

  static bool isFlushBasedInstr(InstrType it) {
    return it == FlushInstr || it == FlushFenceInstr;
  }

  static bool isUsedInstr(InstrType it) { return it != None; }

  static bool isNonVarInstr(InstrType it) {
    return it == PfenceInstr || it == IpInstr;
  }

  static bool isVarInstr(InstrType it) {
    return it == WriteInstr || isFlushBasedInstr(it);
  }

  static bool isWriteInstr(InstrType it) { return it == WriteInstr; }

  template <typename Globals>
  static auto getInstrType(Instruction* i, Globals& globals) {
    if (auto* si = dyn_cast<StoreInst>(i)) {
      return WriteInstr;
    } else if (auto* ii = dyn_cast<InvokeInst>(i)) {
      return IpInstr;
    } else if (auto* ci = dyn_cast<CallInst>(i)) {
      auto* callee = ci->getCalledFunction();

      if (globals.functions.isPfenceFunction(callee)) {
        return PfenceInstr;
      } else if (globals.functions.isVfenceFunction(callee)) {
        return VfenceInstr;
      } else if (globals.functions.isFlushFunction(callee)) {
        return FlushInstr;
      } else if (globals.functions.isFlushFenceFunction(callee)) {
        return FlushFenceInstr;
      } else if (globals.functions.isStoreFunction(callee)) {
        return WriteInstr;
      } else if (globals.functions.isSkippedFunction(callee)) {
        return None;
      } else {
        return IpInstr;
      }
    }

    return None;
  }
};

} // namespace llvm