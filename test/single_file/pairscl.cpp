#include "annot.h"

struct Scl {
  int data;
  sentinel(scl-Scl::data) int valid;

  void nvm_fnc correct() {
    data = 1;
    vfence();
    valid = 1;
    vfence();
  }

  void nvm_fnc correctCircular() {
    valid = 1;
    vfence();
    data = 1;
    vfence();
    valid = 1;
  }

  void nvm_fnc writeValid() {
    data = 1;
    valid = 1;
  }

  void nvm_fnc writeIfValid() {
    data = 1;
    if (data == 1) {
      valid = 1;
    }
  }

  void nvm_fnc correctBranch(bool useNvm) {
    data = 1;
    if (useNvm) {
      vfence();
      valid = 1;
    }
  }

  void correctWriteData() { data = 1; }

  void nvm_fnc wrongIp(bool useNvm) {
    if (useNvm) {
      correctWriteData();
    }
    valid = 1;
  }

  void skip_fnc skip() {
    data = 1;
    pm_clflushopt(&data);
    valid = 1;
    pfence();
    valid = 1;
  }
};

void nvm_fnc recursion(Scl* scl) {
  if (scl->data == 1) {
    scl->valid = 1;
  } else {
    scl->data--;
    recursion(scl);
  }
}
