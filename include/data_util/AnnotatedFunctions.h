#pragma once
#include "Common.h"
#include "FunctionSet.h"

namespace llvm {

class AnnotatedFunctions : public FunctionSet {
  const char* functionAnnotation;

protected:
  bool sameAnnotation(StringRef annotation) const {
    return annotation.equals(functionAnnotation);
  }

public:
  AnnotatedFunctions(const char* functionAnnotation_)
      : functionAnnotation(functionAnnotation_) {}

  virtual void insertAnnotatedFunction(Function* f, StringRef annotation) {
    if (sameAnnotation(annotation)) {
      fs.insert(f);
    }
  }

  void print(raw_ostream& O) const {
    O << functionAnnotation << ": ";
    FunctionSet::print(O);
    O << "\n";
  }
};

} // namespace llvm