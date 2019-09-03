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

  Variable* varLhs;
  ParsedVariable pvLhs;

  Variable* varRhs;
  ParsedVariable pvRhs;

  InstrInfo() : instrType(None) {}

  InstrInfo(Instruction* instr_, InstrType instrType_, Variable* varLhs_,
            ParsedVariable pvLhs_, Variable* varRhs_, ParsedVariable pvRhs_)
      : instr(instr_), instrType(instrType_), varLhs(varLhs_), pvLhs(pvLhs),
        varRhs(varRhs_), pvRhs(pvRhs_) {
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

  auto* getVariable(bool lhs = true) {
    if (lhs) {
      assert(varLhs);
      return varLhs;
    } else {
      assert(varRhs);
      return varRhs;
    }
  }

  auto getParsedVarInfo(bool lhs = true) const {
    if (lhs) {
      assert(pvLhs);
      return pvLhs;
    } else {
      assert(pvRhs);
      return pvRhs;
    }
  }

  auto* getInstruction() {
    assert(instr);
    return instr;
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

    if (varLhs)
      name += " " + varLhs->getName();

    if (varRhs)
      name += " " + varRhs->getName();
    return name;
  }

  bool isUsedInstr() const { return instrType != None; }

  bool isFlushBasedInstr() {
    return instrType == FlushInstr || instrType == FlushFenceInstr;
  }

  static bool isFlushBasedInstr(InstrType it) {
    return it == FlushInstr || it == FlushFenceInstr;
  }

  static bool isUsedInstr(InstrType it) { return it != None; }
};

} // namespace llvm