#pragma once
#include "Common.h"
#include "llvm/Demangle/Demangle.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"

#include "StructField.h"
#include "data_util/FunctionSet.h"

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

class DbgInfo {
  // used for a temporary variable's type

  void addLocalVariables(FunctionSet& funcSet) {
    for (auto* f : funcSet) {
      for (auto& I : instructions(*f)) {
        if (auto* dvi = dyn_cast<DbgValueInst>(&I)) {
          auto* val = dvi->getValue();
          auto* var = dvi->getVariable();
          assert(val && var);
          localVarNames[val] = var;
        }
      }
    }
  }

  void addFunctionNames() {
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

  void addFieldDbgInfo(const DIType* T, int idx, StringRef& typeName) {
    // get debug info
    StringRef realName = T->getName();
    assert(!realName.empty());
    StringRef fileName = T->getFilename();
    assert(!fileName.empty());
    unsigned lineNo = T->getLine();
    assert(lineNo);

    // find element to add info
    auto strIdx = StructField::getIdxName(typeName, idx);

    if (!fieldIdxStrMap.count(strIdx))
      return;

    if (auto* field = fieldIdxStrMap[strIdx]) {
      field->addDbgInfo(realName, fileName, lineNo);
      auto nameStr = field->getStrName();
      fieldNameStrMap[nameStr] = field;
    }
  }

  void addFieldDbgInfo(const DICompositeType* ST) {
    StringRef typeName = ST->getName();
    DINodeArray elements = ST->getElements();
    int idx = 0;
    for (auto element : elements) {
      // get field
      if (auto* E = dyn_cast<DIType>(element)) {
        // get field data
        addFieldDbgInfo(E, idx, typeName);
      }
      ++idx;
    }
  }

  void addFieldDbgInfo() {
    for (const DIType* T : finder.types()) {
      if (auto* ST = dyn_cast<DICompositeType>(T)) {
        StringRef typeName = ST->getName();
        if (typeName.empty()) {
          continue;
        }
        addFieldDbgInfo(ST);
      }
    }
  }

  auto* addField(StructType* st, int idx, Type* ft) {
    auto [fPtr, added] = fields.emplace(st, idx, ft);
    if (!added || fPtr == fields.end())
      report_fatal_error("element not added");
    auto* field = (StructField*)&(*fPtr);
    return field;
  }

  void addInitFieldInfo(StructType* st, int idx, Type* ft) {
    auto* field = addField(st, idx, ft);
    fieldMap[st].push_back(field);

    auto idxName = field->getIdxName();
    fieldIdxStrMap[idxName] = field;
  }

  void addFieldTypeInfo(std::set<StructType*>& trackTypes) {
    for (auto* st : M.getIdentifiedStructTypes()) {
      // only gather data about used types
      if (!trackTypes.count(st))
        continue;

      // add fields
      int idx = 0;
      for (auto* ft : st->elements()) {
        addInitFieldInfo(st, idx, ft);
        ++idx;
      }
    }
  }

  void addTypeFields(std::set<StructType*>& trackTypes) {
    // string are used temporarily to map dbg info to struct type
    addFieldTypeInfo(trackTypes);
    addFieldDbgInfo();
  }

  Module& M;

  // llvm debug info parser
  DebugInfoFinder finder;

  // function names: mangled->real
  std::set<std::string> demangledFunctions;
  std::map<StringRef, StringRef> functionNames;

  // variable infos
  std::set<StructField> fields;
  std::map<StructType*, std::vector<StructField*>> fieldMap;
  std::map<std::string, StructField*> fieldIdxStrMap;
  std::map<std::string, StructField*> fieldNameStrMap;

  // variable names
  std::map<Value*, DILocalVariable*> localVarNames;

public:
  DbgInfo(Module& M_) : M(M_) {
    // function names
    finder.processModule(M);
    addFunctionNames();
  }

  // function related---------------------------------

  StringRef getFunctionName(StringRef mangledName) {
    assert(functionNames.count(mangledName));
    return functionNames[mangledName];
  }

  bool functionExists(StringRef mangledName) const {
    return functionNames.count(mangledName) > 0;
  }

  void printFunctionNames(raw_ostream& O) const {
    O << "Unmangled Names\n";
    O << "---------------\n";

    for (auto& [mangledName, realName] : functionNames) {
      O << "\"" << mangledName << "\"=\"" << realName << "\", ";
    }
    O << "\n\n";
  }

  // var related--------------------------------------

  void addDbgInfoFunctions(FunctionSet& funcSet,
                           std::set<StructType*>& trackTypes) {
    addTypeFields(trackTypes);
    addLocalVariables(funcSet);
  }

  auto* getStructField(StructType* st, int idx) {
    assert(st && idx >= 0);
    auto fieldIdxStr = StructField::getIdxName(st, idx);
    assert(fieldIdxStrMap.count(fieldIdxStr));
    auto* sf = fieldIdxStrMap[fieldIdxStr];
    return sf;
  }

  auto* getStructField(std::string& fieldNameStr) {
    assert(!fieldNameStr.empty());
    assert(fieldNameStrMap.count(fieldNameStr));
    auto* sf = fieldNameStrMap[fieldNameStr];
    return sf;
  }

  auto& getFieldMap(StructType* st) {
    assertInDs(fieldMap, st);
    return fieldMap[st];
  }

  void print(raw_ostream& O) const {
    O << "Global Debug Info\n";
    O << "-----------------\n";

    O << "Struct types and their fields---\n";
    for (auto& [st, fields] : fieldMap) {
      O << st->getName() << ": ";
      int c = 0;
      for (auto& field : fields) {
        O << c << ")" << field->getName() << " ";
        c++;
      }
      O << "\n";
    }

    O << "Local Variable names-----------\n";
    for (auto& [v, lv] : localVarNames) {
      O << *v << ": " << lv->getName() << "\n";
    }
    O << "\n\n";
  }
};

} // namespace llvm