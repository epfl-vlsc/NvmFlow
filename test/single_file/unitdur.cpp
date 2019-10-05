#include "annot.h"

struct Data {
  int data;
  Data* next;
};

struct DurObj {
  dur_field Data* valid;

  void nvm_fnc correct() {
    auto* ptr = new Data();
    pm_flush(ptr);
    pfence();
    valid = ptr;
  }

  void nvm_fnc fenceNotFlushedData(Data* ptr) { valid = ptr; }

  void nvm_fnc correctParam(Data* ptr) {
    pm_flushfence(ptr);
    valid = ptr;
  }

  void nvm_fnc correctObj() {
    auto* data = new Data();
    data->data = 5;

    pm_flushfence(data);
    valid = data;
  }

  void nvm_fnc branchFlush(Data* ptr) {
    if (ptr->data == 1)
      pm_flushfence(ptr);
    valid = ptr;
  }
};

struct DurInt {
  dur_field int* valid;

  void nvm_fnc correct() {
    int* ptr = new int(1);
    pm_flushfence(ptr);
    pfence();
    valid = ptr;
  }

  void nvm_fnc fenceNotFlushedData(int* ptr) { valid = ptr; }

  void nvm_fnc correctParam(int* ptr) {
    pm_flushfence(ptr);
    valid = ptr;
  }

  void nvm_fnc branchFlush(int* ptr) {
    if (cond())
      pm_flushfence(ptr);
    valid = ptr;
  }

  void nvm_fnc doubleFlush(int* ptr) {
    if (cond())
      pm_flushfence(ptr);
    pm_flushfence(ptr);
    valid = ptr;
  }
};
