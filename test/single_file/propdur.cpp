#include "annot.h"

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
