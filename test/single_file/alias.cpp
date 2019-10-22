#include "annot.h"
#include <iostream>

using namespace std;

int* newint(){
  //return new int;
  return pm_malloc();
}

int* f(int* c) {
  *c = 5;
  return c;
}

int nvm_fnc main() {
  int* a = newint();
  int* b = newint();
  int* c = newint();

  *a = 4;
  *b = 6;

  c = f(a);

  pm_flush(a);
  pm_flush(b);
  pm_flush(c);
  cout << a << b << c;

  return 0;
}