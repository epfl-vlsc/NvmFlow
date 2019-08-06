#pragma once

#include "util/Headers.h"

#define DBGMODE

namespace llvm {

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