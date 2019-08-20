#pragma once
#include "Common.h"

#include "../ds/Variable.h"
#include "Lattice.h"

namespace llvm {

using LatVar = Variable*;
using LatVal = Lattice;
using AbstractState = std::map<LatVar, LatVal>;

} // namespace llvm