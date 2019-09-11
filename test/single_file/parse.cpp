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

    c = 5;
    pm_clflush(&c);

    d = new int;
    pm_clflush(&d);

    *b = 5;
    pm_clflush(b);

    *d = 5;
    pm_clflush(d);
  }
};

struct B {
  A a;
  A* b;
  log_field A c;
  log_field A* d;

  void up() {
    a = c;
    pm_clflush(&a);

    b = new A;
    pm_clflush(&b);

    c = *d;
    pm_clflush(&c);

    d = new A;
    pm_clflush(&d);

    *b = *d;
    pm_clflush(b);

    *d = *b;
    pm_clflush(d);

    a.up();

    b->up();

    c.up();

    d->up();
  }
};