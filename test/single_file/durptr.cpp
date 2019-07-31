#include "annot.h"

struct Dur {
  dur_field int* valid;

  void nvm_fnc correct() {
    int* ptr = new int(1);
    clflushopt(ptr);
    pfence();
    valid = ptr;
  }

  void nvm_fnc fenceNotFlushedData(int* ptr) { valid = ptr; }

  void nvm_fnc correctParam(int* ptr) {
    clflush(ptr);
    valid = ptr;
  }

  void nvm_fnc branchFlush(int* ptr) {
    if (*valid == 1)
      clflush(ptr);
    valid = ptr;
  }
};
