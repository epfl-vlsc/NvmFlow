#include "annot.h"
#include <iostream>

using namespace std;

int ab() { return cond(); }

int cd() { return cond(); }

auto fnc() {
  if (cond())
    return ab;
  else
    return cd;
}

int nvm_fnc main() {
  auto f = fnc();
  cout << f();

  return 0;
}