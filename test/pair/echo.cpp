#include "annot.h"
#include <cstdlib>
extern unsigned int flush_range(const void *addr, const size_t size);

#define PM_EQU(pm_dst, y) pm_dst = y;

#define kp_flush_range(addr, size, use_nvm) do {  \
	if (use_nvm) {                                \
		flush_range(addr, size);                  \
	}                                             \
} while (0)

struct kp_kv_master_struct {
  int id;
  int* kv;
  int mode;

  sentinel() int state;
};

typedef struct kp_kv_master_struct kp_kv_master;

int nvm_fnc kp_kv_master_create(kp_kv_master** master, bool use_nvm) {
  PM_EQU(((*master)->id), (1));
  PM_EQU(((*master)->mode), (1));

  kp_flush_range(*master, sizeof(kp_kv_master), use_nvm);
  PM_EQU(((*master)->state), (1));
  kp_flush_range(&((*master)->state), sizeof(int), use_nvm);
  return 0;
}
