#pragma once

#include "util/Headers.h"

#define DBGMODE

namespace llvm {

struct DbgInstr {
  static auto getSourceLocation(Instruction* instruction,
                                bool fullPath = false) {
    assert(instruction);
    auto& debugInfo = instruction->getDebugLoc();
    auto* dbgLoc = debugInfo.get();

    std::string name;
    if (!dbgLoc)
      return name;

    name.reserve(100);
    if (fullPath)
      name += debugInfo->getDirectory().str() + "/";

    name += debugInfo->getFilename().str() + ":";

    int line = debugInfo->getLine();
    name += std::to_string(line);

    if (fullPath) {
      int column = debugInfo->getColumn();
      name += ":" + std::to_string(column);
    }

    return name;
  }

  static void printLocation(Value* v, raw_ostream& O) {
    if (auto* i = dyn_cast<Instruction>(v)) {
      O << getSourceLocation(i) << " " << *i;
    } else if (auto* bb = dyn_cast<BasicBlock>(v)) {
      O << "  bb:" << bb;
    } else if (auto* f = dyn_cast<Function>(v)) {
      O << "  function:" << f->getName();
    } else if (v == nullptr) {
      O << "  0x:";
    }
  }
};

template <typename State> void printState(State& state) {
  for (auto& [latVar, latVal] : state) {
    errs() << "\t" << latVar->getName() << " " << latVal.getName() << "\n";
  }
}

template <typename FuncResults>
void printFunctionResults(FuncResults& functionResults) {
  for (auto& [location, state] : functionResults) {
    DbgInstr::printLocation(location, errs());
    errs() << "\n";
    printState(state);
  }
}

auto getTypeName(Type* type) {
  static const int StructNo = 13;
  static const std::string TypeNames[] = {
      "void",     "float16",   "float32",  "float64",   "float80", "float128",
      "float128", "labels",    "metadata", "mmxvector", "token",   "int",
      "func",     "structure", "array",    "pointer",   "simd"};

  auto tid = type->getTypeID();
  int typeNo = (int)tid;
  if (typeNo == StructNo) {
    assert(isa<StructType>(type));
    return type->getStructName().str();
  }

  assert(typeNo >= 0);
  return TypeNames[typeNo];
}

template <typename Map, typename Key> void assertInDs(Map& map, Key& key) {
  if (!map.count(key)) {
    errs() << "Assert failed for " << key.getName() << "\n";
  }

  assert(map.count(key));
}

template <typename Map, typename Key> void assertInDs(Map& map, Key*& key) {
  if (!map.count(key)) {
    errs() << "Assert failed for " << key->getName() << "\n";
  }

  assert(map.count(key));
}

template <typename Map, typename Key> void assertInDs(Map*& map, Key*& key) {
  if (!map->count(key)) {
    errs() << "Assert failed for " << key->getName() << "\n";
  }

  assert(map->count(key));
}

} // namespace llvm