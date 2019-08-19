#pragma once
#include "Common.h"
#include "llvm/Demangle/Demangle.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"

#include "StructElement.h"

namespace llvm {

std::string demangleFunctionName(Function* f) {
  std::string name;
  char buf[200];
  size_t n;
  int s;
  std::string fncNameStr = f->getName().str();
  const char* fncNameCstr = fncNameStr.c_str();

  itaniumDemangle(fncNameCstr, buf, &n, &s);
  
  if (!s) {
    // successfully demangle
    name = buf;
    size_t found = name.find("(");
    assert(found != std::string::npos && found > 0);

    name = name.substr(0, found);
    
    return name;
  } else {
    name = fncNameStr;
    return name;
  }
}

// used for a temporary variable's type
using IdxStrToElement = std::map<std::string, StructElement*>;

class DbgInfo {
  DebugInfoFinder finder;

  // mangled to real name
  std::set<std::string> demangledFunctions;
  std::map<StringRef, StringRef> functionNames;

  // each field or cls, use addr of these
  std::set<StructElement> elements;

  // auxilliary helpers
  std::map<std::string, StructElement*> fieldToElement;

  std::map<StructElement*, std::set<StructElement*>> fieldMap;

  void initFunctionNames() {
    for (auto* f : finder.subprograms()) {
      auto realName = f->getName();
      auto mangledName = f->getLinkageName();

      if (mangledName.empty())
        mangledName = realName;

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

    for (auto& F : M) {
      if (F.isDeclaration() && !F.isIntrinsic()) {
        auto mangledName = F.getName();
        auto realNameStr = demangleFunctionName(&F);
        auto [sit, _] = demangledFunctions.insert(realNameStr);
        StringRef realName(*sit);
        functionNames[mangledName] = realName;
      }
    }
  }

  void initFieldNames(IdxStrToElement& idxStrMap) {
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
    auto strIdx = StructElement::getAbsoluteName(typeName, idx);

    // type and fields should exist
    if (!idxStrMap.count(strIdx)) {
      return;
    }

    // assert(idxStrMap.count(strIdx));
    auto* element = idxStrMap[strIdx];
    if (!element)
      return;
    element->addDbgInfo(realName, fileName, lineNo);

    // helper
    auto fieldStrIdx = StructElement::getAbsoluteName(typeName, realName);
    assert(!fieldToElement.count(fieldStrIdx));
    fieldToElement[fieldStrIdx] = element;
  }

  void initFields(const DICompositeType* ST, IdxStrToElement& idxStrMap) {
    // add st
    int idx = StructElement::OBJ_ID;
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

  auto* addElement(IdxStrToElement& idxStrMap, StructType* st, int idx,
                   Type* ft) {
    auto [ePtr, added] = elements.emplace(st, idx, ft);
    if (!added || ePtr == elements.end())
      report_fatal_error("element not added");
    auto* element = (StructElement*)&(*ePtr);
    auto strIdx = StructElement::getAbsoluteName(st, idx);
    idxStrMap[strIdx] = element;
    return element;
  }

  void initTypes(IdxStrToElement& idxStrMap) {
    for (auto* st : M.getIdentifiedStructTypes()) {
      // todo dont include known std types
      if (st->getName().startswith("struct._"))
        continue;

      // add st
      int idx = StructElement::OBJ_ID;
      auto* obj = addElement(idxStrMap, st, idx, st);
      fieldMap[obj];

      // add fields
      for (auto* ft : st->elements()) {
        ++idx;
        auto* field = addElement(idxStrMap, st, idx, ft);
        fieldMap[obj].insert(field);
      }
    }
  }

  Module& M;

public:
  DbgInfo(Module& M_) : M(M_) {
    // must do initVariableNames outside the constructor for efficiency

    // function names
    finder.processModule(M);
    initFunctionNames();

    // field and type names
    IdxStrToElement idxStrMap;
    initTypes(idxStrMap);
    initFieldNames(idxStrMap);
  }

  StringRef getFunctionName(StringRef mangledName) {
    assert(functionNames.count(mangledName));
    return functionNames[mangledName];
  }

  bool functionExists(StringRef mangledName) const {
    return functionNames.count(mangledName) > 0;
  }

  auto* getStructElement(std::string& name) {
    assert(fieldToElement.count(name) && "wrong annotation");
    return fieldToElement[name];
  }

  auto* getStructElement(StructElement& tempSe) {
    assertInDs(elements, tempSe);
    auto eIt = elements.find(tempSe);
    auto* se = (StructElement*)&(*eIt);
    return se;
  }

  auto* getStructElement(StructType* st, int idx) {
    StructElement tempSe{st, idx};
    return getStructElement(tempSe);
  }

  auto getVariableName(Instruction* i) const {
    assert(isa<StoreInst>(i) || isa<LoadInst>(i));
    return nullptr;
  }

  auto* getStructElement(StructType* st) {
    StructElement tempSe{st, StructElement::OBJ_ID};
    return getStructElement(tempSe);
  }

  auto* getStructElementFromType(Type* type, int idx) {
    if (auto* st = dyn_cast<StructType>(type)) {
      StructElement tempSe{st, idx};
      return getStructElement(tempSe);
    }
    return (StructElement*)nullptr;
  }

  auto* getStructObj(StructElement* se) {
    auto tempObjSe = se->getObj();
    assertInDs(elements, tempObjSe);
    auto eIt = elements.find(tempObjSe);
    auto* objSe = (StructElement*)&(*eIt);
    return objSe;
  }

  auto& getFieldMap(StructElement* se) {
    assertInDs(fieldMap, se);
    return fieldMap[se];
  }

  void print(raw_ostream& O) const {
    O << "Debug Info\n";
    O << "function names: ";
    for (auto& [mangledName, realName] : functionNames) {
      O << "|" << mangledName << "=" << realName << "|,";
    }
    O << "\n";

    O << "type to fields: ";
    for (auto& [st, fields] : fieldMap) {
      O << "|" << st->getName() << "-->";
      for (auto& field : fields) {
        O << field->getName() << ",";
      }
      O << "|,";
    }
    O << "\n\n";
  }
};

} // namespace llvm