#include "annot.h"
#include <vector>
#include <iostream>
extern void pmem_persist(void* ptr, size_t len, bool usefence);
extern void pmemalloc_activate(void* ptr){}
#define pmalloc malloc
#define PM_EQU(x,y) x = y

namespace storage{
// Persistent list for storing pointers

template<typename V>
class plist {
public:
  struct node {
    struct node* next;
    V val;
  };

  struct node** head;
  struct node** tail;
  bool activate;
  sentinel(storage::plist::tail) off_t _size = 0;

  off_t push_back(V val) {
    struct node* tailp = nullptr;
    struct node* np =
        (struct node*)pmalloc(sizeof(struct node)); // new struct node;

    // Link it in at the end of the list
    PM_EQU((np->val), (val));
    PM_EQU((np->next), (nullptr));

    tailp = (*tail);
    PM_EQU(((*tail)), (np));

    PM_EQU((tailp->next), (np));
    //pm_flushfence(this);
    pm_flushfence(&tailp->next);

    PM_EQU((_size), (_size + 1));
    return 0;
  }
/*
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

    if ((*head) == nullptr) {
      if (init(val) != nullptr)
        index = 0;
      return index;
    }

    struct node* tailp = nullptr;
    struct node* np =
        (struct node*)pmalloc(sizeof(struct node)); // new struct node;

    // Link it in at the end of the list
    PM_EQU((np->val), (val));
    PM_EQU((np->next), (nullptr));

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
*/
  

};

void nvm_fnc x(){
    plist<char*> lol;
    char* a = (char*)"sds";
    lol.push_back(a);
}


}