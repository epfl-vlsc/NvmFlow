#include "annot.h"

extern int x();

struct Dcl {
  int data;
  sentinel(Dcl::data) int valid;

  void nvm_fnc correct() {
    data = 1;
    pm_flush(&data);
    pfence();
    valid = 1;
    pm_flush(&valid);
    pfence();
  }

  void nvm_fnc notFinalizeValid() {
    data = 1;
    pm_flush(&data);
    pfence();
    valid = 1;
    pm_flush(&valid);
  }

  void nvm_fnc writeValidFirst() {
    valid = 1;
    pm_flush(&valid);
    pfence();
    data = 1;
    pm_flush(&data);
    pfence();
  }

  void nvm_fnc fenceNotFlushedData() {
    data = 1;
    pfence();
    valid = 1;
  }

  void nvm_fnc doubleFlush() {
    data = 1;
    pm_flushfence(&data);
    valid = 1;
    pm_flushfence(&data);
  }

  void nvm_fnc doubleLoopFlush() {
    while (valid == 1){
      data = 1;
      pm_flushfence(&data);
    }
    pm_flushfence(&data);
    valid = 1;
  }

  void nvm_fnc writeUncommittedData() {
    data = 1;
    pm_flush(&data);
    data = 1;
    pfence();
    valid = 1;
  }

  void nvm_fnc correctBranch(bool useNvm) {
    data = 1;
    if (useNvm) {
      pm_flush(&data);
      pfence();
      valid = 1;
      pm_flushfence(&valid);
    }
  }

  void nvm_fnc validNotCommitted(bool useNvm) {
    data = 1;
    pm_flushfence(&data);
    valid = 1;
    pm_flush(&valid);
  }

  void nvm_fnc branchNoFence(bool useNvm) {
    data = 1;
    if (useNvm) {
      pm_flush(&data);
    }
    valid = 1;
  }

  void correctWriteData() { data = 1; }

  void nvm_fnc wrongIp(bool useNvm) {
    correctWriteData();
    if (useNvm) {
      pm_flush(&data);
    }
    valid = 1;
  }

  void skip_fnc skip() {
    data = 1;
    pm_flush(&data);
    valid = 1;
    pfence();
    valid = 1;
  }
};

void nvm_fnc recursion(Dcl* dcl) {
  if (dcl->data == 1) {
    dcl->data--;
    recursion(dcl);
  } else {
    dcl->valid = 1;
  }
}
