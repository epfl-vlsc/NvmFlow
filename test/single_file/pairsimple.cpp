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
};

void nvm_fnc correct(Dcl* dcl) {
  dcl->data = 1;
  clflushopt(&dcl->data);
  pfence();
  dcl->valid = 1;
  clflushopt(&dcl->valid);
  pfence();
}
