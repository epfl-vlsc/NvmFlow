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
