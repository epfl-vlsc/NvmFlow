#pragma once
#include "Common.h"

#include "../ds/Variable.h"
#include "Lattice.h"

namespace llvm {

using LatVar = const Variable*;
using LatVal = Lattice;
using AbstractState = std::map<LatVar, LatVal>;

void printAbstractState(AbstractState& state, raw_ostream& O) {
  O << "state\n";
  for (auto& [latVar, latVal] : state) {
    O << latVar->getName() << "-" << latVal.getName() << "\n";
  }
}

} // namespace llvm