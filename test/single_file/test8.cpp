#include "annot.h"

int main() {
  int* ptr = new int(1);
  if (cond()) {
    pm_flushfence(ptr);
    pfence();
    return 0;
  }

  pm_flushfence(ptr);
  return 0;
}
