#include "annot.h"

struct DurInt {
  dur_field int* valid;

  void x(int* a) { pm_flushfence(a); }

  void nvm_fnc correct() {
    int* ptr = (int*)pm_malloc(sizeof(int));
    if (cond())
      x(ptr);
    else
      pm_flushfence(ptr);
    valid = ptr;
  }
};
