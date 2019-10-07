#pragma once
#include "Common.h"

namespace llvm {

std::string stripTemplateStr(StringRef& typeName) {
  // todo not cfg parsing
  static constexpr const char* Lsep = "<";
  static constexpr const char* Rsep = ">";
  static constexpr const size_t Last = std::string::npos;

  auto typeStr = typeName.str();
  size_t lp = 0, rp = 0;
  bool isTemplated = true;
  int c = 2;
  do {
    lp = typeStr.find(Lsep);
    rp = typeStr.find(Rsep);
    isTemplated = (lp != Last && rp != Last);
    if (isTemplated)
      typeStr = typeStr.substr(0, lp) + typeStr.substr(rp + 1, Last);
    c--;
  } while (isTemplated && c > 0);

  return typeStr;
}

std::string stripStructStr(StructType* st) {
  static constexpr const char* CSEP = ".";

  auto stName = st->getName();
  auto clsAnnotName = stripTemplateStr(stName);

  size_t skipChars = clsAnnotName.find(CSEP);
  auto clsName = clsAnnotName.substr(skipChars + 1);

  return clsName;
}

class StructFieldBase {
protected:
  StructType* st;
  int idx;

  StructFieldBase(StructType* st_, int idx_) : st(st_), idx(idx_) {
    assertField(st, idx);
  }

  virtual ~StructFieldBase() {}

public:
  auto getStType() const { return st; }

  auto getInfo() { return std::pair(st, idx); }

  bool operator<(const StructFieldBase& X) const {
    return std::tie(st, idx) < std::tie(X.st, X.idx);
  }

  bool operator==(const StructFieldBase& X) const {
    return st == X.st && idx == X.idx;
  }
};

class StructField : public StructFieldBase {
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
    name += st->getName().str() + "->";
    name += fieldName.str();
    return name;
  }

  void print(raw_ostream& O) const { O << getName(); }

  auto getStrName() const {
    std::string name;
    name.reserve(150);
    auto clsName = stripStructStr(st);
    name += clsName + "::" + fieldName.str();
    return name;
  }
};

} // namespace llvm