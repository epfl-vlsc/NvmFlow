#include "annot.h"

struct Obj {
  int data;
  sentinel(Obj::data) int valid;

  void nvm_fnc correct() {
    data = 1;
    clflushopt(this);
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

  void writeUncommittedData() {
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

  void correctWriteData(){
    data = 1;
  }

  void wrongIpa(bool useNvm) {
    correctWriteData();
    if (useNvm) {
      clflush(&data);
    }
    valid = 1;
  }
};
