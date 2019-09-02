#include "annot.h"

extern int x();

struct Dcl {
  int data;
  sentinel(Dcl::data) int valid;

  void nvm_fnc correct() {
    data = 1;
    pm_clflushopt(&data);
    pfence();
    valid = 1;
    pm_clflushopt(&valid);
    pfence();
  }

  void nvm_fnc notFinalizeValid() {
    data = 1;
    pm_clflushopt(&data);
    pfence();
    valid = 1;
    pm_clflushopt(&valid);
  }

  void nvm_fnc writeValidFirst() {
    valid = 1;
    pm_clflushopt(&valid);
    pfence();
    data = 1;
    pm_clflushopt(&data);
    pfence();
  }

  void nvm_fnc fenceNotFlushedData() {
    data = 1;
    pfence();
    valid = 1;
  }

  void nvm_fnc doubleFlush() {
    data = 1;
    pm_clflush(&data);
    valid = 1;
    pm_clflush(&data);
  }

  void nvm_fnc doubleLoopFlush() {
    while (valid == 1){
      data = 1;
      pm_clflush(&data);
    }
    pm_clflush(&data);
    valid = 1;
  }

  void nvm_fnc writeUncommittedData() {
    data = 1;
    pm_clflushopt(&data);
    data = 1;
    pfence();
    valid = 1;
  }

  void nvm_fnc correctBranch(bool useNvm) {
    data = 1;
    if (useNvm) {
      pm_clflushopt(&data);
      pfence();
      valid = 1;
      pm_clflush(&valid);
    }
  }

  void nvm_fnc validNotCommitted(bool useNvm) {
    data = 1;
    pm_clflush(&data);
    valid = 1;
    pm_clflushopt(&valid);
  }

  void nvm_fnc branchNoFence(bool useNvm) {
    data = 1;
    if (useNvm) {
      pm_clflushopt(&data);
    }
    valid = 1;
  }

  void correctWriteData() { data = 1; }

  void nvm_fnc wrongIp(bool useNvm) {
    correctWriteData();
    if (useNvm) {
      pm_clflushopt(&data);
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

void nvm_fnc recursion(Dcl* dcl) {
  if (dcl->data == 1) {
    dcl->data--;
    recursion(dcl);
  } else {
    dcl->valid = 1;
  }
}
