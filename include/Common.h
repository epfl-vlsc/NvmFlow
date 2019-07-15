#pragma once

#include "util/Headers.h"

#define DBGMODE

namespace llvm {

auto getSourceLocation(Instruction* instruction) {
  auto& debugInfo = instruction->getDebugLoc();
  std::string name;
  name.reserve(50);
  name += debugInfo->getDirectory().str() + "/";
  name += debugInfo->getFilename().str() + ":";
  int line = debugInfo->getLine();
  int column = debugInfo->getColumn();

  name += std::to_string(line) + ":" + std::to_string(column);
  return name;
}

void printLocation(Value* v, raw_ostream& O) {
  if (auto* i = dyn_cast<Instruction>(v)) {
    O << *i;
  } else if (auto* bb = dyn_cast<BasicBlock>(v)) {
    O << "bb:" << bb;
  } else if (auto* f = dyn_cast<Function>(v)) {
    O << "function:" << f->getName();
  } else if (v == nullptr) {
    O << "0x:";
  }


}

} // namespace llvm