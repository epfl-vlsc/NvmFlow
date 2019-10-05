#include "annot.h"

struct Obj {
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
    data = 1;
    while (valid == 1){
      data = 1;
      pm_flushfence(this);
    }
    pm_flushfence(this);
    valid = 1;
    pm_flushfence(&valid);
  }

  void nvm_fnc writeUncommittedData() {
    data = 1;
    pm_flush(this);
    data = 1;
    pfence();
    valid = 1;
    pm_flushfence(&valid);
  }

  void nvm_fnc correctBranch(bool useNvm) {
    data = 1;
    if (useNvm) {
      pm_flush(this);
      pfence();
      valid = 1;
      pm_flushfence(&valid);
    }
  }

  void nvm_fnc correctBranchNoFence(bool useNvm) {
    data = 1;
    if (useNvm) {
      pm_flushfence(this);
    }
    valid = 1;
    pm_flushfence(&valid);
  }

  void correctWriteData() { data = 1; }

  void nvm_fnc correctIp(bool useNvm) {
    correctWriteData();
    if (useNvm) {
      pm_flushfence(this);
    }
    valid = 1;
    pm_flushfence(&valid);
  }

  void skip_fnc skip() {
    data = 1;
    pm_flush(this);
    valid = 1;
    pfence();
    valid = 1;
    pm_flushfence(&valid);
  }
};

void nvm_fnc objRecursion(Obj* dcl) {
  if (dcl->data == 1) {
    dcl->data--;
    objRecursion(dcl);
  } else {
    dcl->valid = 1;
  }
}
/*
struct Chain {
  int data;
  sentinel(Dcl::data) int valid;
  sentinel(Dcl::valid) int valid2;

  void nvm_fnc correct() {
    data = 1;
    vfence();
    valid = 1;
    pm_flushfence(&valid);
    valid2 = 1;
  }

  void nvm_fnc wrongCircular() {
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
    data = 1;
    pm_flushfence(&data);
    valid = 1;
    pm_flushfence(&valid);
    valid2 = 1;
    pm_flushfence(&valid);
  }
};

struct Dcl {
  struct Obj{int v;};

  int data;
  sentinel() Obj valid;

  void nvm_fnc correct() {
    data = 1;
    pm_clflushopt(this);
    pfence();
    valid = {1};
    pm_clflushopt(&valid);
    pfence();
  }

  void nvm_fnc fenceNotFlushedData() {
    data = 1;
    pfence();
    valid = {1};
  }

  void nvm_fnc doubleFlush() {
    data = 1;
    pm_clflush(this);
    valid = {1};
    pm_clflush(this);
  }

  void nvm_fnc doubleLoopFlush() {
    while (valid == 1){
      data = 1;
      pm_clflush(this);
    }
    pm_clflush(this);  
  }

  void nvm_fnc writeUncommittedData() {
    data = 1;
    pm_clflushopt(this);
    data = 1;
    pfence();
    valid = 1;
  }

  void nvm_fnc correctBranch(bool useNvm) {
    data = 1;
    if (useNvm) {
      pm_clflushopt(this);
      pfence();
    }
    valid = 1;
  }

  void nvm_fnc branchNoFence(bool useNvm) {
    data = 1;
    if (useNvm) {
      pm_clflushopt(this);
    }
    valid = 1;
  }

  void correctWriteData() { data = 1; }

  void nvm_fnc wrongIp(bool useNvm) {
    correctWriteData();
    if (useNvm) {
      pm_clflushopt(this);
    }
    valid = 1;
  }

  void skip_fnc skip() {
    data = 1;
    pm_clflushopt(this);
    valid = 1;
    pfence();
    valid = 1;
  }
};
*/