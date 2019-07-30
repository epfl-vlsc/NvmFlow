#pragma once

#include "util/Headers.h"

#define DBGMODE

namespace llvm {

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