#include "annot.h"
#define PM_EQU(x, y) x = y

extern void flush_range(void* ptr);

struct vector {
  void** data;              // array of void pointers
  unsigned long long size;  // number of array elements allocated
  unsigned long long count; // number of elements in use
  bool use_nvm;
  sentinel() int state;
};

int nvm_fnc vector_append(vector* v) {
  PM_EQU((v->state), (1));
  flush_range(&(v->state));
  PM_EQU((v->size), (1));
  flush_range(&(v->data));

  /* Use two flushes here to make this "repeatable" - if we fail after
   * the first set + flush, there are no real effects. */
  PM_EQU((v->data[v->count]), new long);
  flush_range(&(v->data[v->count]));

  /* Do we need a memory fence right here? Only if we're flushing (so
   * the fence is already internal in kp_flush_range()); otherwise,
   * we're not concerned about anybody else seeing the count and the
   * element out-of-order... (right?). */
  PM_EQU((v->count), (v->count + 1));
  flush_range((void*)&(v->count));

  return 0;
}