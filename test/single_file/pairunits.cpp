#include "annot.h"

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
};

void nvm_fnc correct(Dcl* dcl) {
  dcl->data = 1;
  pm_flush(&dcl->data);
  pfence();
  dcl->valid = 1;
  pm_flush(&dcl->valid);
  pfence();
}
