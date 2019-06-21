#include "annot.h"

struct Dcl {
  sentinelp(dcl-Dcl::valid) int data;
  int valid;

  void correct() {
    data = 1;
    clflushopt(&data);
    pfence();
    valid = 1;
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
