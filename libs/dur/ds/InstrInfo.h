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
  VarInfo* varInfo;

  Variable* lhsAlias;
  Variable* rhsAlias;

  InstrInfo() : instrType(None) {}

  InstrInfo(Instruction* instr_, InstrType instrType_, VarInfo* varInfo_,
            Variable* lhsAlias_, Variable* rhsAlias_)
      : instr(instr_), instrType(instrType_), varInfo(varInfo_),
        lhsAlias(lhsAlias_), rhsAlias(rhsAlias_) {
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

  auto* getVarInfo() {
    assert(varInfo);
    return varInfo;
  }

  auto* getVariable() {
    assert(lhsAlias);
    return lhsAlias;
  }

  auto* getRhsAlias() {
    assert(rhsAlias);
    return rhsAlias;
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

    return name;
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

  static bool isWriteInstr(InstrType it) { return it == WriteInstr; }
};

} // namespace llvm