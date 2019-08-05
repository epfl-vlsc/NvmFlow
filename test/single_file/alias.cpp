#include "annot.h"

struct A {
  log_field A* a;
  log_field A* b;
};

void __attribute__((noinline)) m1(A* a) { 
  if(a->b == a->a){
    a->b = a; 
  }else{
    a->a = a; 
  }
}

void m3(A* a, A* c) { 
  A* b = a;
  b->a = c;
  m1(b);
}