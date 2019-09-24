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

  auto getName() const {
    assert(instr);
    std::string name;
    name.reserve(100);
    name += Strs[(int)instrType];

    auto sl = DbgInstr::getSourceLocation(instr);
    if (!sl.empty())
      name += " " + sl;

    if (var)
      name += " " + var->getName();
    return name;
  }

  bool isUsedInstr() const { return instrType != None; }

  bool isLogBasedInstr() const { return instrType == LoggingInstr; }

  static bool isUsedInstr(InstrType it) { return it != None; }

  static bool isNonVarInstr(InstrType it) {
    return it == TxBegInstr || it == TxEndInstr || it == IpInstr;
  }

  static bool isWriteInstr(InstrType it) { return it == WriteInstr; }
};

} // namespace llvm