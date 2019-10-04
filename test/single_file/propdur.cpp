#include "annot.h"

struct Dur {
  dur_field int* next;

  void nvm_fnc correct() {
    auto* ptr = new int(5);
    pm_flush(ptr);
    pfence();
    next = ptr;
  }

  void nvm_fnc commitPtr() {
    auto* ptr = new int(5);
    if (cond())
      pm_flushfence(ptr);

    next = ptr;
  }

  void nvm_fnc doubleFlush() {
    auto* ptr = new int(5);
    if (cond())
      pm_flushfence(ptr);
    pm_flushfence(ptr);
    next = ptr;
  }
};
