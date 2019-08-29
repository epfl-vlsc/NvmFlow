#pragma once
#include "Common.h"

namespace llvm {

class AnnotParser {
  static constexpr const char* FIELD_ANNOT = "pair";
  static constexpr const char* SCL_ANNOT = "scl";
  static constexpr const char* SEP = "-";

public:
  static auto parseAnnotation(StringRef annotation) {
    bool useDcl = annotation.contains(SCL_ANNOT) ? false : true;
    auto [_, name] = annotation.rsplit(SEP);
    return std::pair(name.str(), useDcl);
  }

  static auto isValidAnnotation(StringRef annotation) {
    return annotation.contains(FIELD_ANNOT);
  }
};

} // namespace llvm