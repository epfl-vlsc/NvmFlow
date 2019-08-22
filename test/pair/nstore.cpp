#include "annot.h"
#include <cstdlib>

#define PM_EQU(pm_dst, y) pm_dst = y;

extern void* pmalloc(size_t size);

namespace storage {

template <typename V> class plist {
public:
  struct node {
    struct node* next;
    V val;
  };

  struct node** head;
  struct node** tail;
  bool activate;
  sentinel() off_t _size = 0;

  plist() : head(NULL), tail(NULL), activate(true) {
    head = (struct node**)pmalloc(sizeof(struct node*));
    tail = (struct node**)pmalloc(sizeof(struct node*));
    PM_EQU(((*head)), (NULL));
    PM_EQU(((*tail)), (NULL));
  }

  plist(void** _head, void** _tail) {
    PM_EQU((head), ((struct node**)_head));
    PM_EQU((tail), ((struct node**)_tail));
    PM_EQU((activate), (true));
  }

  plist(void** _head, void** _tail, bool _activate) {
    PM_EQU((head), ((struct node**)_head));
    PM_EQU((tail), ((struct node**)_tail));
    PM_EQU((activate), (_activate));
  }

  struct node* init(V val) {
    struct node* np =
        (struct node*)pmalloc(sizeof(struct node)); // new struct node;

    PM_EQU((np->next), ((*head)));
    PM_EQU((np->val), (val));

    PM_EQU(((*head)), (np));
    PM_EQU(((*tail)), (np));

    if (activate)
      pmemalloc_activate(np);

    PM_EQU((_size), (_size + 1));
    return np;
  }

  off_t nvm_fnc push_back(V val) {
    off_t index = -1;

    if ((*head) == NULL) {
      if (init(val) != NULL)
        index = 0;
      return index;
    }

    struct node* tailp = NULL;
    struct node* np =
        (struct node*)pmalloc(sizeof(struct node)); // new struct node;

    // Link it in at the end of the list
    PM_EQU((np->val), (val));
    PM_EQU((np->next), (NULL));

    tailp = (*tail);
    PM_EQU(((*tail)), (np));

    if (activate)
      pmemalloc_activate(np);

    PM_EQU((tailp->next), (np));
    pmem_persist(&tailp->next, sizeof(*np), 0);

    index = _size;
    PM_EQU((_size), (_size + 1));
    return index;
  }
};

} // namespace storage
