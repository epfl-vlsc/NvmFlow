#include "annot.h"
#define PM_EQU(x, y) x = y 

extern void flush_range(void* ptr);

struct kp_vte {
  /* The snapshot stored in the kp_vte is either the local worker's
   * current snapshot when they put the value ("start timestamp") OR
   * the snapshot when the local worker performed the merge into the
   * master ("commit timestamp").
   */
  const void* value; // pointer to the actual data
  int size;          // size of the value
  int lvn;           // local version number
  int ttl;           // number of snapshots that reference this version
  int snapshot;      // LOCAL only: working snapshot when inserted!
  int* cr;           // MASTER only: pointer to commit record!
  sentinel() int state;
};

int nvm_fnc kp_vte_create(kp_vte** vte, const void* value, int size, int lvn,
                          int ttl, int snapshot, int* cr, bool use_nvm) {
  /* Here it is: a tombstone value for a deletion is represented by a
   * vte with a NULL value pointer and 0 size.
   */
  PM_EQU(((*vte)->value), (value));
  PM_EQU(((*vte)->size), (size));
  PM_EQU(((*vte)->lvn), (lvn));
  PM_EQU(((*vte)->ttl), (ttl));
  PM_EQU(((*vte)->snapshot), (snapshot));
  PM_EQU(((*vte)->cr), (cr));

  /* "CDDS": flush, set state, and flush again. The flushes will only actually
   * occur if use_nvm is true. */
  flush_range(vte);
  PM_EQU(((*vte)->state), (1));
  flush_range(&((*vte)->state));
  return 0;
}
