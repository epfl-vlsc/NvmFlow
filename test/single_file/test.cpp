#include "annot.h"

volatile int* val;

struct Dur {
  int data;
  dur_field int* next;

  void nvm_fnc correct() {
    auto* ptr = new int(2);
    clflushopt(ptr);
    pfence();
    next = ptr;
  }

};
