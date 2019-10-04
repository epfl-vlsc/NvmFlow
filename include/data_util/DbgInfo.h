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
        Value* val = nullptr;
        DILocalVariable* var = nullptr;

        // get debug information
        if (auto* dvi = dyn_cast<DbgValueInst>(&I)) {
          val = dvi->getValue();
          var = dvi->getVariable();
        } else if (auto* ddi = dyn_cast<DbgDeclareInst>(&I)) {
          val = ddi->getAddress();
          var = ddi->getVariable();
        }

        if (!val)
          continue;

        assert(var);

        auto* type = val->getType();
        if (type->isPointerTy())
          localVarNames[val] = var;
      }
    }
  }

  void addFunctionNames(FunctionSet& funcSet) {
    std::set<std::string> funcSetNames;
    for (auto& F : M) {
      if (!funcSet.count(&F))
        continue;

      auto funcName = F.getName().str();
      funcSetNames.insert(funcName);

      if (F.isDeclaration() && !F.isIntrinsic()) {
        auto mangledName = funcName;
        auto realNameStr = demangleFunctionName(&F);
        auto [sit, _] = demangledFunctions.insert(realNameStr);
        StringRef realName(*sit);
        functionNames[mangledName] = realName.str();
      }
    }

    for (auto* f : finder.subprograms()) {
      auto realName = f->getName();
      auto mangledName = f->getLinkageName();

      if (mangledName.empty())
        mangledName = realName;

      if (!funcSetNames.count(mangledName))
        continue;

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

      functionNames[mangledName.str()] = realName.str();
    }
  }

  void addFieldDbgInfo(const DIType* T, int idx, StringRef& typeName) {
    // get debug info
    StringRef realName = T->getName();
    if (realName.empty())
      return;

    StringRef fileName = T->getFilename();
    if (fileName.empty())
      return;

    unsigned lineNo = T->getLine();
    if (!lineNo)
      return;

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

  void addFieldTypeInfo(std::set<StructType*>& structTypes) {
    for (auto* st : M.getIdentifiedStructTypes()) {
      // only gather data about used types
      if (!structTypes.count(st))
        continue;

      // add fields
      int idx = 0;
      for (auto* ft : st->elements()) {
        addInitFieldInfo(st, idx, ft);
        ++idx;
      }
    }
  }

  void addTypeFields(std::set<StructType*>& structTypes) {
    // string are used temporarily to map dbg info to struct type
    addFieldTypeInfo(structTypes);
    addFieldDbgInfo();
  }

  void addTrackedTypes(std::set<Type*>& trackTypes) {
    trackedTypes = std::move(trackTypes);
  }

  Module& M;

  // llvm debug info parser
  DebugInfoFinder finder;

  // function names: mangled->real
  std::set<std::string> demangledFunctions;
  std::map<std::string, std::string> functionNames;

  // variable infos
  std::set<StructField> fields;
  std::map<StructType*, std::vector<StructField*>> fieldMap;
  std::map<std::string, StructField*> fieldIdxStrMap;
  std::map<std::string, StructField*> fieldNameStrMap;

  // type infos
  std::set<Type*> trackedTypes;

  // variable names
  std::map<Value*, DILocalVariable*> localVarNames;

public:
  DbgInfo(Module& M_) : M(M_) {
    // function names
    finder.processModule(M);
  }

  // function related---------------------------------

  auto getFunctionName(Function* f) {
    auto mangledName = f->getName().str();
    assertInDs(functionNames, mangledName);
    return functionNames[mangledName];
  }

  // var related--------------------------------------
  bool isUsedStructType(StructType* st) const { return fieldMap.count(st); }

  bool isTrackedType(Type* type) const { return trackedTypes.count(type); }

  void addDbgInfoFunctions(FunctionSet& funcSet, std::set<Type*>& trackTypes,
                           std::set<StructType*>& structTypes) {
    addFunctionNames(funcSet);
    addTrackedTypes(trackTypes);
    addTypeFields(structTypes);
    addLocalVariables(funcSet);
  }

  auto* getStructField(StructType* st, int idx) {
    assertField(st, idx);
    auto fieldIdxStr = StructField::getIdxName(st, idx);
    assertInDs(fieldIdxStrMap, fieldIdxStr);
    auto* sf = fieldIdxStrMap[fieldIdxStr];
    return sf;
  }

  auto* getStructField(std::string& fieldNameStr) {
    assert(!fieldNameStr.empty());
    if(!fieldNameStrMap.count(fieldNameStr))
      report_fatal_error("wrong annotation");
    assertInDs(fieldNameStrMap, fieldNameStr);
    auto* sf = fieldNameStrMap[fieldNameStr];
    return sf;
  }

  auto& getFieldMap(StructType* st) {
    assertInDs(fieldMap, st);
    return fieldMap[st];
  }

  auto* getDILocalVariable(Value* v) {
    if (localVarNames.count(v)) {
      return localVarNames[v];
    }

    return (DILocalVariable*)nullptr;
  }

  void print(raw_ostream& O) const {
    O << "Global Debug Info\n";
    O << "-----------------\n";
    int c = 0;

    O << "Function Names\n";
    O << "---------------\n";
    for (auto& [mangledName, realName] : functionNames) {
      O << "\"" << mangledName << "\"=\"" << realName << "\", ";
    }
    O << "\n\n";

    O << "Tracked Types\n";
    O << "-------------\n";
    c = 0;
    for (auto* trackedType : trackedTypes) {
      O << c << "(" << *trackedType << "),";
      c++;
    }
    O << "\n\n";

    O << "Struct types and their fields\n";
    O << "-----------------------------\n";
    for (auto& [st, fields] : fieldMap) {
      O << st->getName() << ": ";
      int c = 0;
      for (auto& field : fields) {
        O << c << ")" << field->getName() << " ";
        c++;
      }
      O << "\n\n";
    }

    O << "Local Variable names sample\n";
    O << "---------------------------\n";
    c = 0;
    for (auto& [v, lv] : localVarNames) {
      O << *v << ": " << lv->getName() << "\n";
      if (c >= 5)
        break;
      c++;
    }
    O << "\n\n";
  }
};

} // namespace llvm