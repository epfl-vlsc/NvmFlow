#pragma once
#include "Common.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"

namespace llvm {

class VariableDbgInfo {
  StringRef typeName;
  StringRef fieldName;
  StringRef fileName;
  unsigned lineNo;

public:
  VariableDbgInfo(StringRef typeName_, StringRef fieldName_,
                  StringRef fileName_, unsigned lineNo_)
      : typeName(typeName_), fieldName(fieldName_), fileName(fileName_),
        lineNo(lineNo_) {}

  VariableDbgInfo() {}

  void print(raw_ostream& O) const {
    O << fileName << ":" << lineNo << ":" << typeName << "::" << fieldName;
  }

  std::string getName() const {
    return (fileName + ":" + std::to_string(lineNo) + ":" + typeName +
            "::" + fieldName)
        .str();
  }
};

class DbgInfo {
  DebugInfoFinder finder;
  std::map<StringRef, StringRef> functionNames;
  std::map<std::string, VariableDbgInfo> variableNames;

  void initFunctionNames() {
    for (auto* f : finder.subprograms()) {
      auto realName = f->getName();
      auto mangledName = f->getLinkageName();

      // check skip
      auto fileNameRef = f->getFilename();
      auto fileName = fileNameRef.str();
      auto lineNo = f->getLine();
      auto isDefinition = f->isDefinition();
      bool skipFnc = (mangledName.empty() || fileNameRef.empty() ||
                      lineNo == 0 || !isDefinition);
      if (skipFnc) {
        continue;
      }

      functionNames[mangledName] = realName;
    }
  }

  void initVariableNames() {
    for (const DIType* T : finder.types()) {
      if (auto* CT = dyn_cast<DICompositeType>(T)) {
        StringRef typeName = T->getName();
        if (typeName.empty()) {
          continue;
        }

        initFields(CT, typeName);
      }
    }
  }

  void initFields(const DICompositeType* CT, const StringRef& typeName) {
    DINodeArray elements = CT->getElements();

    int i = -1;
    for (auto element : elements) {
      ++i;

      // get field
      if (auto* E = dyn_cast<DIType>(element)) {
        // get field data
        StringRef fieldName = E->getName();
        StringRef fileName = E->getFilename();
        unsigned lineNo = E->getLine();

        auto fullFieldName = typeName + ":" + std::to_string(i);

        // is a valid field
        if (lineNo == 0) {
          continue;
        }

        variableNames[fullFieldName.str()] =
            VariableDbgInfo(typeName, fieldName, fileName, lineNo);
      }
    }
  }

public:
  DbgInfo(Module& M) {
    finder.processModule(M);
    initFunctionNames();
    initVariableNames();
  }

  StringRef getFunctionName(StringRef mangledName) {
    assert(functionNames.count(mangledName));
    return functionNames[mangledName];
  }

  auto getVariableName(std::string& variableInfo) {
    assert(variableNames.count(variableInfo));
    return variableNames[variableInfo];
  }

  bool functionExists(StringRef mangledName) const {
    return functionNames.count(mangledName) > 0;
  }

  void print(raw_ostream& O) const {
    O << "Debug Info\n";
    O << "function names: ";
    for (auto& [mangledName, realName] : functionNames) {
      O << mangledName << "-" << realName << ",";
    }
    O << "\n";

    O << "variable names: ";
    for (auto& [variableName, info] : variableNames) {
      O << variableName << "-" << info.getName() << ",";
    }
    O << "\n";
  }
};

} // namespace llvm