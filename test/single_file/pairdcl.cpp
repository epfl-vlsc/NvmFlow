#include "annot.h"

struct Dcl {
  int data;
  sentinel(Dcl::data) int valid;

  void nvm_fnc correct() {
    data = 1;
    clflushopt(&data);
    pfence();
    valid = 1;
    clflushopt(&valid);
    pfence();
  }

  void nvm_fnc correctCircular() {
    valid = 1;
    clflush(&valid);
    data = 1;
    clflushopt(&data);
    pfence();
    valid = 1;
  }

  void nvm_fnc fenceNotFlushedData() {
    data = 1;
    pfence();
    valid = 1;
  }

  void nvm_fnc doubleFlush() {
    data = 1;
    clflush(&data);
    valid = 1;
    clflush(&data);
  }

  void nvm_fnc doubleLoopFlush() {
    while (valid == 1){
      data = 1;
      clflush(&data);
    }
    clflush(&data);  
  }

  void nvm_fnc writeUncommittedData() {
    data = 1;
    clflushopt(&data);
    data = 1;
    pfence();
    valid = 1;
  }

  void nvm_fnc correctBranch(bool useNvm) {
    data = 1;
    if (useNvm) {
      clflushopt(&data);
      pfence();
      valid = 1;
    }
  }

  void nvm_fnc branchNoFence(bool useNvm) {
    data = 1;
    if (useNvm) {
      clflush(&data);
    }
    valid = 1;
  }

  void correctWriteData() { data = 1; }

  void nvm_fnc wrongIp(bool useNvm) {
    correctWriteData();
    if (useNvm) {
      clflush(&data);
    }
    valid = 1;
  }

  void skip_fnc skip() {
    data = 1;
    clflushopt(&data);
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
