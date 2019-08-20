#include "annot.h"
#include <cstdlib>

//__attribute__((noinline))

struct A {
  A* a;
  A* b;
};

void m1(A* a, A* b) {
  b = new A;
  a = b;
}

struct Dur {
  dur_field int* next;

  void nvm_fnc correct() {
    auto* ptr = new int(5);
    clflushopt(ptr);
    pfence();
    next = ptr;
  }
};
