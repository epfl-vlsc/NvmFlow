#include "annot.h"

struct Dcl {
  int data;
  sentinel(Dcl::data) int valid;

  void nvm_fnc correct() {
    data = 1;
    pm_flushfence(&data);
    valid = 1;
    pm_flushfence(&valid);
  }

  void nvm_fnc commitPairBug() {
    data = 1;
    pm_flush(&data);
    valid = 1;
    pm_flushfence(&valid);
  }

  void nvm_fnc doubleFlush() {
    data = 1;
    if (cond())
      pm_flushfence(&data);
    pm_flushfence(&data);
    valid = 1;
    pm_flushfence(&valid);
  }

  void nvm_fnc volatileSentinelBug() {
    data = 1;
    pm_flushfence(&data);
    valid = 1;
  }
};