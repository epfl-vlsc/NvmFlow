#include "annot.h"

struct Data {
  int data;
  Data* next;
};

struct Dur {
  dur_field Data* next;

  void nvm_fnc correct() {
    auto* ptr = new Data();
    pm_clflushopt(ptr);
    pfence();
    next = ptr;
  }

  void nvm_fnc fenceNotFlushedData(Data* ptr) { next = ptr; }

  void nvm_fnc correctParam(Data* ptr) {
    pm_clflush(ptr);
    next = ptr;
  }

  void nvm_fnc correctObj() {
    auto* data = new Data();
    data->data = 5;

    pm_clflush(data);
    next = data;
  }

  void nvm_fnc writeObj() {
    auto* data = new Data();
    data->data = 5;

    pm_clflush(data);
    data->data = 5;

    next = data;
  }

  void nvm_fnc branchFlush(Data* ptr) {
    if (ptr->data == 1)
      pm_clflush(ptr);
    next = ptr;
  }
};
