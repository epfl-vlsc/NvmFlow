#include "annot.h"

struct A {
  int a;
  int* b;
  log_field int c;
  log_field int* d;

  void up() {
    a = 5;
    pm_clflush(&a);

    b = new int;
    pm_clflush(&b);
    *b = 5;
    pm_clflush(b);

    c = 5;
    pm_clflush(&c);

    d = new int;
    pm_clflush(&d);
    *d = 5;
    pm_clflush(d);
  }
};

struct B {
  A a;

  void local(A* b) {
    A* c = new A;
    a.a = 5;
    b->a = 5;
    c->a = 5;
  }

  void up() {
    a.a = 5;
    pm_clflush(&a.a);

    a.b = new int;
    pm_clflush(&a.b);
    *a.b = 5;
    pm_clflush(a.b);
  }

  void local(A** b, A** c) {
    *b = *c;
  }
};