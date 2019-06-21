#include "annot.h"

struct Ptr {
  int data;
  p_ptr Ptr* next;

  void correct() {
    Ptr* ptr = new Ptr;
    clflush(ptr);
    this->next = ptr;
  }

  void notFlushed() {
    Ptr* ptr = new Ptr;
    Ptr* ptr2 = new Ptr;
    clflush(ptr);
    ptr = ptr2;
    this->next = ptr;
  }

  void notFlushed2() {
    Ptr* ptr = new Ptr;
    this->next = ptr;
  }

  void writeParam(Ptr* param, Ptr* param2){
    param->next = param2;    
  }

  void correctParam(Ptr* param, Ptr* param2){
    clflush(param2);
    param->next = param2;
    clflush(param);
    clflush(&this->next);
    this->next = param;
  }  
};
