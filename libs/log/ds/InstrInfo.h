#pragma once
#include "Common.h"
#include "Variable.h"
#include "parser_util/InstrParser.h"

namespace llvm {

struct InstrInfo {
  enum InstrType {
    WriteInstr,
    LoggingInstr,
    TxBegInstr,
    TxEndInstr,
    IpInstr,
    None
  };

  static constexpr const char* Strs[] = {"write",  "logging", "tx begin",
                                         "tx end", "ip",      "none"};
  Instruction* instr;
  InstrType instrType;
  Variable* var;
  ParsedVariable pv;

  InstrInfo() : instrType(None) {}

  InstrInfo(Instruction* instr_, InstrType instrType_, Variable* var_,
            ParsedVariable& pv_)
      : instr(instr_), instrType(instrType_), var(var_), pv(pv_) {
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

  auto* getVariable() {
    assert(var);
    return var;
  }

  auto getParsedVarInfo() const {
    assert(pv.isUsed());
    return pv;
  }

  auto* getInstruction() {
    assert(instr);
    return instr;
  }

  void print(raw_ostream& O) const{
    assert(instr);
    
    O << Strs[(int)instrType];

    auto sl = DbgInstr::getSourceLocation(instr);
    if (!sl.empty())
      O << " " << sl;

    if (var)
      O << " " << var->getName();
    
    O << "\n";
  }

  bool isUsedInstr() const { return instrType != None; }

  bool isLogBasedInstr() const { return instrType == LoggingInstr; }

  static bool isUsedInstr(InstrType it) { return it != None; }

  static bool isNonVarInstr(InstrType it) {
    return it == TxBegInstr || it == TxEndInstr || it == IpInstr;
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

      if (globals.functions.isLoggingFunction(callee)) {
        return LoggingInstr;
      } else if (globals.functions.isTxBeginFunction(callee)) {
        return TxBegInstr;
      } else if (globals.functions.isTxEndFunction(callee)) {
        return TxEndInstr;
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