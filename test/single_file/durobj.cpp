#include "annot.h"

struct Data {
  int data;
  Data* next;
};

struct Dur {
  dur_field Data* valid;

  void nvm_fnc correct() {
    auto* ptr = new Data();
    clflushopt(ptr);
    pfence();
    valid = ptr;
  }

  void nvm_fnc fenceNotFlushedData(Data* ptr) { valid = ptr; }

  void nvm_fnc correctParam(Data* ptr) {
    clflush(ptr);
    valid = ptr;
  }

  void nvm_fnc correctObj() {
    auto* data = new Data();
    data->data = 5;

    clflush(data);
    valid = data;
  }

  void nvm_fnc writeObj() {
    auto* data = new Data();
    data->data = 5;

    clflush(data);
    data->data = 5;

    valid = data;
  }

  void nvm_fnc branchFlush(Data* ptr) {
    if (ptr->data == 1)
      clflush(ptr);
    valid = ptr;
  }
};
