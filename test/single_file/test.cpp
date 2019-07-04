#include "annot.h"

void __attribute__((noinline)) flush(void* ptr) { (*(int*)ptr) = 0; }

struct Bobj {
  __attribute((annotate("cann"))) int c;
  __attribute((annotate("cpann"))) int* cp;

  Bobj() : c(0), cp(nullptr) {}

  void setc(int z) {
    c = z;
    cp = &z;
  }

  int __attribute__((noinline)) loadc() { return c; }

  int* __attribute__((noinline)) loadcp() { return cp; }
};

struct Aobj {
  __attribute((annotate("aann"))) int a;
  __attribute((annotate("apann"))) int* ap;
  __attribute((annotate("bann"))) Bobj b;
  __attribute((annotate("bpann"))) Bobj* bp;

  void __attribute__((noinline)) store() {
    a = 1;
    *ap = 1;
    b.setc(1);
    bp->setc(1);
    bp = &b;
  }
};

void fs(Aobj* o) {
  flush(&o->b.c);
}

