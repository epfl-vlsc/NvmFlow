#pragma once
#include "Common.h"
#include "parser_util/FunctionParser.h"

namespace llvm {

template <typename Globals> class FuncParser : public FunctionParser<Globals> {
  using Base = FunctionParser<Globals>;

  static constexpr const char* skipNames[] = {
      "pmemobj_direct", "pmemobj_pool_by_oid",
      "_ZN18tree_map_node_toidC2Ev", "_ZN18tree_map_node_toidC2E7pmemoid"};

  bool isKnownSkipFunction(Function* f) const override {
    for (auto* skipName : skipNames) {
      if (f->getName().contains(skipName))
        return true;
    }

    return false;
  }

public:
  FuncParser(Module& M_, Globals& globals_) : Base(M_, globals_) {}
};

} // namespace llvm