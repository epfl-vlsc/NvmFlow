#include "annot.h"

struct Dcl {
  int data;
  sentinel() int valid;

  void nvm_fnc correct() {
    data = 1;
    pm_flush(this);
    pfence();
    valid = 1;
    pm_flush(&valid);
    pfence();
  }

  void nvm_fnc fenceNotFlushedData() {
    data = 1;
    pfence();
    valid = 1;
  }

  void nvm_fnc doubleFlush() {
    data = 1;
    pm_flushfence(this);
    valid = 1;
    pm_flushfence(this);
  }

  void nvm_fnc doubleLoopFlush() {
    while (valid == 1){
      data = 1;
      pm_flushfence(this);
    }
    pm_flushfence(this);  
  }

  void nvm_fnc writeUncommittedData() {
    data = 1;
    pm_flush(this);
    data = 1;
    pfence();
    valid = 1;
  }

  void nvm_fnc correctBranch(bool useNvm) {
    data = 1;
    if (useNvm) {
      pm_flush(this);
      pfence();
    }
    valid = 1;
  }

  void nvm_fnc branchNoFence(bool useNvm) {
    data = 1;
    if (useNvm) {
      pm_flush(this);
    }
    valid = 1;
  }

  void correctWriteData() { data = 1; }

  void nvm_fnc wrongIp(bool useNvm) {
    correctWriteData();
    if (useNvm) {
      pm_flush(this);
    }
    valid = 1;
  }

  void skip_fnc skip() {
    data = 1;
    pm_flush(this);
    valid = 1;
    pfence();
    valid = 1;
  }
};

void nvm_fnc recursion(Dcl* dcl) {
  if (dcl->data == 1) {
    dcl->valid = 1;
  } else {
    dcl->data--;
    recursion(dcl);
  }
}
