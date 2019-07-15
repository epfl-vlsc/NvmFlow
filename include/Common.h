#pragma once

#include "util/Headers.h"

namespace llvm {

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