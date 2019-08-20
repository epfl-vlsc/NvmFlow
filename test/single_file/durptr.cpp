#include "annot.h"

struct Dur {
  dur_field int* next;
  int* next2;

  void nvm_fnc correct() {
    auto* ptr = new int(5);
    clflushopt(ptr);
    pfence();
    next = ptr;
    next2 = new int(6);
  }

  void nvm_fnc fenceNotFlushedData(int* ptr) { next = ptr; }

  void nvm_fnc correctParam(int* ptr) {
    clflush(ptr);
    next = ptr;
  }

  void nvm_fnc correctInt() {
    auto* data = new int(5);

    clflush(data);
    next = data;
  }

  void nvm_fnc writeInt() {
    auto* data = new int(5);

    clflush(data);
    *data = 5;

    next = data;
  }

  void nvm_fnc branchFlush(int* ptr) {
    if (*ptr == 1)
      clflush(ptr);
    next = ptr;
  }
};
