#include "annot.h"


struct DurInt {
  dur_field int* valid;

  void nvm_fnc main() {
    int* ptr = new int(1);
    pm_flushfence(ptr);
    pfence();
    valid = ((int*)0);
  }
};
