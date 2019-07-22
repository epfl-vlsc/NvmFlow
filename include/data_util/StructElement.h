#pragma once
#include "Common.h"

namespace llvm {

class StructElement {
public:
  static constexpr const int OBJ_ID = -1;

protected:
  StructType* st;
  int idx;

  StructElement(StructType* st_, int idx_) : st(st_), idx(idx_) {
    assert(st && idx >= OBJ_ID);
  }

public:
  auto getStType() const { return st; }

  bool isObj() const { return idx == OBJ_ID; }

  bool isField() const { return idx != OBJ_ID; }

  virtual std::string getName() const {
    return st->getName().str() + "::" + std::to_string(idx);
  }

  virtual void print(raw_ostream& O) const { O << getName(); }

  bool operator<(const StructElement& X) const {
    return st < X.st || idx < X.idx;
  }

  bool operator==(const StructElement& X) const {
    return st == X.st && idx == X.idx;
  }
};

class FullStructElement : public StructElement {
  Type* fieldType;

  StringRef realName;
  StringRef fileName;
  unsigned lineNo;

public:
  FullStructElement(StructType* st_, int idx_, Type* fieldType_)
      : StructElement(st_, idx_), fieldType(fieldType_) {
    assert(fieldType_);
  }

  void addDbgInfo(StringRef realName_, StringRef fileName_, unsigned lineNo_) {
    realName = realName_;
    fileName = fileName_;
    lineNo = lineNo_;
  }

  auto getFullName() const {
    std::string name;
    name.reserve(150);
    name += fileName.str() + ":" + std::to_string(lineNo);
    name += ":" + getName() + "::" + realName.str();
    return name;
  }

  void printFull(raw_ostream& O) const { O << getFullName(); }

  std::string getName() const override {
    return st->getName().str() + "::" + realName.str();
  }

  void print(raw_ostream& O) const { O << getName(); }

  static std::string getAbsoluteName(StructType* st_, int idx_) {
    auto fullClsName = st_->getName().str();
    auto clsName = fullClsName.substr(fullClsName.find(".") + 1);
    return clsName + "::" + std::to_string(idx_);
  }

  static std::string getAbsoluteName(StringRef typeName_, int idx_) {
    return typeName_.str() + "::" + std::to_string(idx_);
  }

  static std::string getAbsoluteName(StringRef typeName_, StringRef realName_) {
    return typeName_.str() + "::" + realName_.str();
  }
};

} // namespace llvm