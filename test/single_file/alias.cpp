#include "annot.h"

struct A {
  log_field int a;
  log_field A* b;
};

void m1(A* a) {
  a->a = 5;
  a->b = a;
}

void m2(A& a) {
  a.a = 5;
  a.b = &a;
}

void m3(A* a) { m1(a); }

void m3(A* a, int b) {
  a->a = b;
  m1(a);
}