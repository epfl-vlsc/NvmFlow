#pragma once
#include "Common.h"
#include "Variable.h"
#include "analysis_util/InstrParser.h"

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
  Variable* var;
  ParsedVariable pv;

  InstrInfo() : instrType(None) {}

  InstrInfo(Instruction* instr_, InstrType instrType_, Variable* var_,
            ParsedVariable pv_)
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

  std::string getVarName() const {
    assert(var);
    return var->getName();
  }

  std::string getSrcLoc() const {
    assert(instr);
    return DbgInstr::getSourceLocation(instr);
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

  static bool isFlushBasedInstr(InstrType instrType) {
    return instrType == FlushInstr || instrType == FlushFenceInstr;
  }

  static bool isUsedInstr(InstrType it) { return it != None; }
};

} // namespace llvm