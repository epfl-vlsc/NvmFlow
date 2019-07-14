#include "annot.h"

struct A {
  log_field A* a;
  log_field A* b;
};

void __attribute__((noinline)) m1(A* a) { a->b = a; }

void m3(A* a) { m1(a); }