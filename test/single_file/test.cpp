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

  void nvm_fnc wrongObj() {
    auto* data = new Data();
    pm_flushfence(data);
    data->data = 5;
    valid = data;
  }

  void nvm_fnc branchFlush(Data* ptr) {
    if (ptr->data == 1)
      pm_flushfence(ptr);
    valid = ptr;
  }
};

