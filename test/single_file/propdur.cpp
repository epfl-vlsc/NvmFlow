#include "annot.h"

struct Data {
  int data;
  Data* next;
};

struct DurObj {
  dur_field Data* valid;

  void nvm_fnc correct() {
    auto* ptr = (Data*)pm_malloc(sizeof(Data));
    pm_flush(ptr);
    pfence();
    valid = ptr;
  }

  void nvm_fnc correctParam(Data* ptr) {
    pm_flushfence(ptr);
    valid = ptr;
  }

  void nvm_fnc correctObj() {
    auto* data = (Data*)pm_malloc(sizeof(Data));
    data->data = 5;

    pm_flushfence(data);
    valid = data;
  }

  void nvm_fnc wrongObj() {
    auto* data = (Data*)pm_malloc(sizeof(Data));
    pm_flushfence(data);
    data->data = 5;
    valid = data;
  }

  void nvm_fnc notPersistentObj() {
    Data* ptr = (Data*)pm_malloc(sizeof(Data));
    pm_flush(ptr);
    valid = ptr;
  }
};

struct DurInt {
  dur_field int* valid;

  void nvm_fnc correct() {
    int* ptr = (int*)pm_malloc(sizeof(int));
    pm_flushfence(ptr);
    valid = ptr;
  }

  void nvm_fnc correctNull() {
    valid = nullptr;
  }

  void nvm_fnc notPersistent() {
    int* ptr = (int*)pm_malloc(sizeof(int));
    pm_flush(ptr);
    valid = ptr;
  }

  void nvm_fnc correctParam(int* ptr) {
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
