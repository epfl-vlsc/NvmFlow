#pragma once
#include "Common.h"

namespace llvm {

class StructFieldBase {
protected:
  StructType* st;
  int idx;

  StructFieldBase(StructType* st_, int idx_) : st(st_), idx(idx_) {
    assert(st && idx >= 0);
  }

  virtual ~StructFieldBase() {}

public:
  auto getStType() const { return st; }

  auto getInfo() { return std::pair(st, idx); }

  bool operator<(const StructFieldBase& X) const {
    return st < X.st || idx < X.idx;
  }

  bool operator==(const StructFieldBase& X) const {
    return st == X.st && idx == X.idx;
  }
};

class StructField : public StructFieldBase {
  static auto stripStructStr(StructType* st_) {
    auto fullClsName = st_->getName().str();
    auto clsName = fullClsName.substr(fullClsName.find(".") + 1);
    return clsName;
  }

  Type* fieldType;

  StringRef fieldName;
  StringRef fileName;
  unsigned lineNo;

public:
  StructField(StructType* st_, int idx_, Type* fieldType_)
      : StructFieldBase(st_, idx_), fieldType(fieldType_) {
    assert(fieldType);
  }

  StructField(StructType* st_, int idx_) : StructFieldBase(st_, idx_) {}

  void addDbgInfo(StringRef fieldName_, StringRef fileName_, unsigned lineNo_) {
    fieldName = fieldName_;
    fileName = fileName_;
    lineNo = lineNo_;
  }

  auto* getFieldType() {
    assert(fieldType);
    return fieldType;
  }

  auto getName() const {
    std::string name;
    name.reserve(50);
    name += fieldName.str();
    return name;
  }

  void print(raw_ostream& O) const { O << getName(); }

  auto getIdxName() const {
    std::string name;
    name.reserve(150);
    auto clsName = stripStructStr(st);
    name += clsName + "::" + std::to_string(idx);
    return name;
  }

  static auto getIdxName(StringRef& typeName, int idx_) {
    std::string name;
    name.reserve(150);
    name += typeName.str() + "::" + std::to_string(idx_);
    return name;
  }

  static auto getIdxName(StructType* st_, int idx) {
    std::string name;
    name.reserve(150);
    auto clsName = stripStructStr(st_);
    name += clsName + "::" + std::to_string(idx);
    return name;
  }

  auto getStrName() const {
    std::string name;
    name.reserve(150);
    auto clsName = stripStructStr(st);
    name += clsName + "::" + fieldName.str();
    return name;
  }
};

} // namespace llvm