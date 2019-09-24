#include "annot.h"

struct Dcl {
  int data;
  sentinel(scl-Dcl::data) int valid;
  sentinel(Dcl::valid) int valid2;

  void nvm_fnc correct() {
    data = 1;
    vfence();
    valid = 1;
    pm_flushfence(&valid);
    valid2 = 1;
  }

  void nvm_fnc correctCircular() {
    valid2 = 1;
    pm_flushfence(&valid2);
    data = 1;
    pm_flushfence(&data);
    valid = 1;
  }

  void nvm_fnc fenceNotFlushedData() {
    data = 1;
    pfence();
    valid = 1;
    valid2 = 1;
  }

  void nvm_fnc doubleFlush() {
    valid = 1;
    pm_flushfence(&valid);
    valid2 = 1;
    pm_flushfence(&valid);
  }
};
