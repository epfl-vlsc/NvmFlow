#include "annot.h"
#include "pthread.h"
#define PM_EQU(x, y) x = y

extern void flush_range(void* ptr);

struct kp_kvstore {
  int id;
  sentinel() int state;
};

void* lol(void *arg) {
  kp_kvstore** kv = (kp_kvstore**)pm_malloc(4);
  PM_EQU(((*kv)->id), (1)); // persistent
  flush_range(*kv);
  PM_EQU(((*kv)->state), (1)); // persist
  flush_range(&((*kv)->state));
}

int nvm_fnc kp_vte_create(kp_kvstore** kv) {
  pthread_t threads[2];
  int rc = pthread_create(&threads[0], 0, &lol, 0);

  return 0;
}
