#include "annot.h"

struct Dcl {
  int data;
  sentinel(dcl-Dcl::data) int valid;

  void nvm_fnc correct() {
    data = 1;
    clflushopt(&data);
    pfence();
    valid = 1;
    clflushopt(&valid);
    pfence();
  }

  void correctCircular() {
    valid = 1;
    clflush(&valid);
    data = 1;
    clflushopt(&data);
    pfence();
    valid = 1;
  }

  void fenceNotFlushedData() {
    data = 1;
    pfence();
    valid = 1;
  }

  void wrongWriteUnFlushedData() {
    data = 1;
    clflushopt(&data);
    data = 1;
    pfence();
    valid = 1;
  }

  void correctBranch(bool useNvm) {
    data = 1;
    if (useNvm) {
      clflushopt(&data);
      pfence();
      valid = 1;
    }
  }

  void branchNoFence(bool useNvm) {
    data = 1;
    if (useNvm) {
      clflush(&data);
    }
    valid = 1;
  }

  void writeData(){
    data = 1;
  }

  void wrongIpa(bool useNvm) {
    writeData();
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
