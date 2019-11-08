#include "annot.h"

void f(int& a);

void f2(int& a) {
  mutateInt(a);
  f(a);
}

void f(int& a) {
  if (cond()) {
    mutateInt(a);
  } else {
    f2(a);
  }
}

int nvm_fnc main() {
  int a = 5;
  f(a);
  mutateInt(a);
  return 0;
}
