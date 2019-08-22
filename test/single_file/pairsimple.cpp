#include "annot.h"

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
};

void nvm_fnc correct(Dcl* dcl) {
  dcl->data = 1;
  pm_clflushopt(&dcl->data);
  pfence();
  dcl->valid = 1;
  pm_clflushopt(&dcl->valid);
  pfence();
}
