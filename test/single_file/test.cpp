#include "annot.h"

struct Data {
  int data;
  Data* next;
};

void pmfs_flush_buffer(void* ptr);

void memcpy_to_nvmm(char* ptr, int offset){
  pmfs_flush_buffer(ptr + offset);
}

struct DurObj {
  dur_field Data* valid;

  void nvm_fnc correct() {
    auto* ptr = new Data();

    int offset = 5;
    char* xm = (char*)ptr;
    memcpy_to_nvmm((char *)ptr, offset);
		pmfs_flush_buffer(xm + offset);
    valid = ptr;
  }
/*
  void nvm_fnc fenceNotFlushedData(Data* ptr) { valid = ptr; }

  void nvm_fnc correctParam(Data* ptr) {
    pm_flushfence(ptr);
    valid = ptr;
  }

  void nvm_fnc correctObj() {
    auto* data = new Data();
    data->data = 5;

    pm_flushfence(data);
    valid = data;
  }

  void nvm_fnc wrongObj() {
    auto* data = new Data();
    pm_flushfence(data);
    data->data = 5;
    valid = data;
  }

  void nvm_fnc branchFlush(Data* ptr) {
    if (ptr->data == 1)
      pm_flushfence(ptr);
    valid = ptr;
  }
  */
};

