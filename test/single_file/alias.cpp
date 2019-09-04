#include "annot.h"
#include <cstdlib>

//__attribute__((noinline))

extern int x();

struct A {
  int a;
  A* b;

  void nvm_fnc m1(A* ptr) {
    while (ptr) {
      ptr->a = 3;
      ptr = ptr->b;
    }
  }
};