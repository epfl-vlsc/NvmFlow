#include "annot.h"

struct Dur {
  dur_field int* next;
  int* next2;

  void nvm_fnc correct() {
    auto* ptr = new int(5);
    pm_clflushopt(ptr);
    pfence();
    next = ptr;
  }

  void nvm_fnc doubleFlush() {
    auto* ptr = new int(5);
    pm_clflushopt(ptr);
    pfence();
    pm_clflushopt(ptr);
    next = ptr;
  }

  void nvm_fnc doubleFlushParam(int* ptr) {
    pm_clflushopt(ptr);
    pfence();
    pm_clflushopt(ptr);
    next = ptr;
  }

  void nvm_fnc correctParam(int* ptr) {
    pm_clflush(ptr);
    next = ptr;
  }

  void nvm_fnc correctInt() {
    auto* data = new int(5);

    pm_clflush(data);
    next = data;
  }

  void nvm_fnc writeInt() {
    auto* data = new int(5);

    pm_clflush(data);
    *data = 5;

    next = data;
  }

  void nvm_fnc branchFlush(int* ptr) {
    if (*ptr == 1)
      pm_clflush(ptr);
    next = ptr;
  }

  void writeIpa(int* ptr) { next = ptr; }

  void nvm_fnc ipa() {
    auto* data = new int(5);
    pm_clflushopt(data);
    writeIpa(data);
  }
};
