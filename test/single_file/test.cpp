#include "annot.h"

void flush(void* ptr) { (*(int*)ptr) = 0; }

struct Bobj {
  int c;
  int* cp;

  Bobj() : c(0), cp(nullptr) {}

  void setc(int z) {
    c = z;
    cp = &z;
  }

  int loadc() { return c; }

  int* loadcp() { return cp; }
};

struct Aobj {
  int a;
  int* ap;
  Bobj b;
  Bobj* bp;

  void store() {
    a = 1;
    *ap = 1;
    b.setc(1);
    bp->setc(1);
    bp = &b;
  }

  void load() {
    int z1 = a;
    int z2 = *ap;
    Bobj z3 = b;
    Bobj z4 = *bp;
    int z5 = b.c;
    int z6 = bp->c;
    int z7 = *(bp->cp);
  }
};

void ff() {
  Aobj* a = new Aobj;
  flush(a);
  flush(&a);
  flush(a->bp);
  flush(&a->bp);
  flush(a->bp->cp);
  flush(&a->bp->cp);
}

void fs(Aobj* o) {
  o->a = 1;
  *(o->ap) = 1;
  o->b.setc(1);
  o->bp->setc(1);
  o->bp = &o->b;
}

void fl(Aobj* obj) {
    Aobj& o = *obj;
  int z1 = o.a;
  int* z2 = o.ap;
  Bobj z3 = o.b;
  Bobj* z4 = o.bp;
  int z5 = o.b.c;
  int z6 = o.bp->c;
  int* z7 = o.bp->cp;
}