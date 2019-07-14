#pragma once
#include "Common.h"

#include "Lattice.h"
#include "data_util/Variable.h"

namespace llvm {

using LatVar = Variable*;
using LatVal = Lattice;
using AbstractState = std::map<LatVar, LatVal>;

} // namespace llvm