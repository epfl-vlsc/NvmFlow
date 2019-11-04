#include "annot.h"

struct DurInt {
  dur_field int* valid;

  /*
    void nvm_fnc fenceNotFlushedData(int* ptr) { valid = ptr; }

    void nvm_fnc correctParam(int* ptr) {
      pm_flushfence(ptr);
      valid = ptr;
    }

    void nvm_fnc branchFlush(int* ptr) {
      if (cond())
        pm_flushfence(ptr);
      valid = ptr;
    }

    void nvm_fnc doubleFlush(int* ptr) {
      if (cond())
        pm_flushfence(ptr);
      pm_flushfence(ptr);
      valid = ptr;
    }
    */
};

int nvm_fnc main() {
  DurInt* dptr = new DurInt;
  int* ptr = new int(1);
  pm_flushfence(ptr);
  pfence();

  dptr->valid = ptr;
  //pm_flushfence(dptr);

  int* ptr2 = new int(1);
  pm_flushfence(ptr2);
  pfence();
  mutate(ptr2);

 
}