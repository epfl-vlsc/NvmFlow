#include "annot.h"

struct A {
  int a;
  int* b;
  log_field int c;
  log_field int* d;

  void up() {
    a = 5;
    pm_flushfence(&a);

    b = new int;
    pm_flushfence(&b);
    *b = 5;
    pm_flushfence(b);

    c = 5;
    pm_flushfence(&c);

    d = new int;
    pm_flushfence(&d);
    *d = 5;
    pm_flushfence(d);
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
    pm_flushfence(&a.a);

    a.b = new int;
    pm_flushfence(&a.b);
    *a.b = 5;
    pm_flushfence(a.b);
  }

  void local(A** b, A** c) {
    *b = *c;
    pm_flushfence(*b);
    pm_flushfence(b);
  }
};

extern bool x();

void checkPhi(A* a, A* b) {
  A* c = nullptr;
  if (x()) {
    int z = x();
    a->a = z;
    c = a;
  } else {
    c = b;
    c->a = x();
  }

  c->b = new int;
}

struct C {
  int a[2];

  void array() {
    a[1] = 5;
    pm_flushfence(&a);
    pm_flushfence(&a[1]);
    pm_flushfence(a);
  }
};

void ptrs(int* a) {
  a = new int(2);
  *a = 5;
  pm_flushfence(&a);
  pm_flushfence(a);

  int* b = new int(2);
  *b = 5;
  pm_flushfence(&b);
  pm_flushfence(b);
}