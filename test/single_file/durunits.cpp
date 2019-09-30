#include "annot.h"

struct Dur {
  dur_field int* next;
  int* next2;

  void nvm_fnc correct() {
    auto* ptr = new int(5);
    pm_flush(ptr);
    pfence();
    next = ptr;
  }

  void nvm_fnc doubleFlush() {
    auto* ptr = new int(5);
    pm_flush(ptr);
    pfence();
    pm_flush(ptr);
    next = ptr;
  }

  void nvm_fnc doubleFlushParam(int* ptr) {
    pm_flush(ptr);
    pfence();
    pm_flush(ptr);
    next = ptr;
  }

  void nvm_fnc correctParam(int* ptr) {
    pm_flushfence(ptr);
    next = ptr;
  }

  void nvm_fnc correctInt() {
    auto* data = new int(5);

    pm_flushfence(data);
    next = data;
  }

  void nvm_fnc writeInt() {
    auto* data = new int(5);

    pm_flushfence(data);
    *data = 5;

    next = data;
  }

  void nvm_fnc branchFlush(int* ptr) {
    if (*ptr == 1)
      pm_flushfence(ptr);
    next = ptr;
  }

  void writeIpa(int* ptr) { next = ptr; }

  void nvm_fnc ipa() {
    auto* data = new int(5);
    pm_flush(data);
    writeIpa(data);
  }
};

struct Data {
  int data;
  Data* next;
};

struct Dur2 {
  dur_field Data* next;

  void nvm_fnc correct() {
    auto* ptr = new Data();
    pm_flushfence(ptr);
    pfence();
    next = ptr;
  }

  void nvm_fnc fenceNotFlushedData(Data* ptr) { next = ptr; }

  void nvm_fnc correctParam(Data* ptr) {
    pm_flushfence(ptr);
    next = ptr;
  }

  void nvm_fnc correctObj() {
    auto* data = new Data();
    data->data = 5;

    pm_flushfence(data);
    next = data;
  }

  void nvm_fnc writeObj() {
    auto* data = new Data();
    data->data = 5;

    pm_flushfence(data);
    data->data = 5;

    next = data;
  }

  void nvm_fnc branchFlush(Data* ptr) {
    if (ptr->data == 1)
      pm_flushfence(ptr);
    next = ptr;
  }
};