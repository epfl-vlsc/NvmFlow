#pragma once
#include "Common.h"
#include "FunctionSet.h"

namespace llvm {

class AnnotatedFunctions : public FunctionSet {
protected:
  const char* annot;

  bool sameAnnotation(StringRef annotation) const {
    return annotation.equals(annot);
  }

public:
  AnnotatedFunctions(const char* annot_)
      : annot(annot_) {}

  virtual void insertAnnotatedFunction(Function* f, StringRef annotation) {
    if (sameAnnotation(annotation)) {
      fs.insert(f);
    }
  }

  void print(raw_ostream& O) const {
    O << annot << ": ";
    FunctionSet::print(O);
    O << "\n";
  }
};

} // namespace llvm