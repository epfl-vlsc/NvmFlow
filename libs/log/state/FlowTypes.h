#pragma once
#include "Common.h"

#include "data_util/VariableInfo.h"

#include "Lattice.h"

namespace llvm::FlowTypes {

using LatVar = VariableInfo*;
using LatVal = Lattice;
using AbstractState = std::map<LatVar, LatVal>;

} // namespace llvm::FlowTypes