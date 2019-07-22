#pragma once
#include "Common.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"

#include "StructElement.h"

namespace llvm {

using IdxStrToElement = std::map<std::string, FullStructElement*>;

class DbgInfo {
  DebugInfoFinder finder;

  //mangled to real name
  std::map<StringRef, StringRef> functionNames;

  //each field or cls, use addr of these
  std::set<FullStructElement> elements;

  //auxilliary helpers
  std::map<std::string, FullStructElement*> fieldToElement;

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

  void initVariableNames(IdxStrToElement& idxStrMap) {
    for (const DIType* T : finder.types()) {
      if (auto* ST = dyn_cast<DICompositeType>(T)) {
        StringRef typeName = ST->getName();
        if (typeName.empty()) {
          continue;
        }

        initFields(ST, idxStrMap);
      }
    }
  }

  void addElementDbgInfo(IdxStrToElement& idxStrMap, const DIType* T, int idx,
                         StringRef& typeName) {
    // get debug info
    StringRef realName = T->getName();
    StringRef fileName = T->getFilename();
    unsigned lineNo = T->getLine();
    if (lineNo == 0) {
      errs() << realName << " " << fileName << " " << lineNo << "\n";
      report_fatal_error("check case");
    }

    // find element to add info
    auto strIdx = FullStructElement::getAbsoluteName(typeName, idx);
    assert(idxStrMap.count(strIdx));
    auto* element = idxStrMap[strIdx];
    element->addDbgInfo(realName, fileName, lineNo);

    //helper
    auto fieldStrIdx = FullStructElement::getAbsoluteName(typeName, realName);
    assert(!fieldToElement.count(fieldStrIdx));
    fieldToElement[fieldStrIdx] = element;
  }

  void initFields(const DICompositeType* ST, IdxStrToElement& idxStrMap) {
    // add st
    int idx = -1;
    StringRef typeName = ST->getName();
    addElementDbgInfo(idxStrMap, ST, idx, typeName);

    DINodeArray elements = ST->getElements();
    for (auto element : elements) {
      ++idx;

      // get field
      if (auto* E = dyn_cast<DIType>(element)) {
        // get field data
        addElementDbgInfo(idxStrMap, E, idx, typeName);
      }
    }
  }

  void addElement(IdxStrToElement& idxStrMap, StructType* st, int idx, Type* ft) {
    auto [ePtr, added] = elements.emplace(st, idx, ft);
    if (!added || ePtr == elements.end())
      report_fatal_error("element not added");
    auto* element = (FullStructElement*)&(*ePtr);
    auto strIdx = FullStructElement::getAbsoluteName(st, idx);
    idxStrMap[strIdx] = element;
  }

  void initTypes(IdxStrToElement& idxStrMap) {
    for (auto* st : M.getIdentifiedStructTypes()) {
      // add st
      int idx = -1;
      addElement(idxStrMap, st, idx, st);

      // add fields
      for (auto* ft : st->elements()) {
        ++idx;
        addElement(idxStrMap, st, idx, ft);
      }
    }
  }

  Module& M;

public:
  DbgInfo(Module& M_) : M(M_) {}

  void initDbgInfo() {
    finder.processModule(M);
    initFunctionNames();

    IdxStrToElement idxStrMap;
    initTypes(idxStrMap);
    initVariableNames(idxStrMap);
  }

  StringRef getFunctionName(StringRef mangledName) {
    assert(functionNames.count(mangledName));
    return functionNames[mangledName];
  }
  /*
    auto getVariableName(std::string& variableInfo) {
      assert(variableNames.count(variableInfo));
      return variableNames[variableInfo];
    }
   */
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

    O << "elements: ";
    for (auto& e : elements) {
      O << e.getName() << ",";
    }
    O << "\n";

    O << "field to element: ";
    for (auto& [f, e] : fieldToElement) {
      O << f << ",";
    }
    O << "\n";
  }
};

} // namespace llvm