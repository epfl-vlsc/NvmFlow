#pragma once
#include "Common.h"

namespace llvm {

class StructElementBase {
public:
  static constexpr const int OBJ_ID = -1;

protected:
  StructType* st;
  int idx;

  StructElementBase(StructType* st_, int idx_) : st(st_), idx(idx_) {
    assert(st && idx >= OBJ_ID);
  }

public:
  auto getStType() const { return st; }

  bool isObj() const { return idx == OBJ_ID; }

  bool isField() const { return idx != OBJ_ID; }

  virtual std::string getName() const = 0;

  virtual void print(raw_ostream& O) const = 0;

  bool operator<(const StructElementBase& X) const {
    return st < X.st || idx < X.idx;
  }

  bool operator==(const StructElementBase& X) const {
    return st == X.st && idx == X.idx;
  }
};

class StructElement : public StructElementBase {
  Type* fieldType;

  StringRef realName;
  StringRef fileName;
  unsigned lineNo;

public:
  StructElement(StructType* st_, int idx_, Type* fieldType_)
      : StructElementBase(st_, idx_), fieldType(fieldType_) {
    assert(fieldType_);
  }

  StructElement(StructType* st_, int idx_) : StructElementBase(st_, idx_) {
    // used for temporary objects to search list
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

  void print(raw_ostream& O) const override { O << getName(); }

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