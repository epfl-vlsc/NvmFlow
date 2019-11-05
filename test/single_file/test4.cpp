#include "annot.h"

struct Dcl {
  int data;
  sentinel(Dcl::data) int valid;

};

void m(Dcl* dcl, Dcl* dcl2){
  Dcl* dcl3 = new Dcl;
  dcl->valid = 1;
  dcl2->valid = 1;
  dcl3->valid = 1;
  dcl->data = 1;
  dcl2->data = 1;
  dcl3->data = 1;
}

int nvm_fnc main() {
  Dcl* dcl = new Dcl;
  Dcl* dcl2 = new Dcl;
  m(dcl, dcl2);
  dcl->valid = 1;
  dcl2->valid = 1;
  dcl->data = 1;
  dcl2->data = 1;
}