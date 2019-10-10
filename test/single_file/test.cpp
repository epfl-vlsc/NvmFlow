#include "annot.h"

/*
struct Dur {
  struct Data {
    int data;
  };
  dur_field Data* next;

  void nvm_fnc correct() {
    auto* ptr = new Data{5};
    pm_flush(ptr);
    pfence();
    next = ptr;
  }

  void nvm_fnc commitPtr() {
    auto* ptr = new Data{5};
    if (cond())
      pm_flushfence(ptr);

    next = ptr;
  }

  void nvm_fnc commitModifyPtr() {
    auto* ptr = new Data{5};
    pm_flushfence(ptr);
    ptr->data = 4;
    next = ptr;
  }

  void nvm_fnc doubleFlush() {
    auto* ptr = new Data{5};
    if (cond())
      pm_flushfence(ptr);
    pm_flushfence(ptr);
    next = ptr;
  }
};
*/

struct node{
  int data;
  node* next;
};

struct tree{
  node* root;
  int size;
};


void zed(tree* t, node* n){
  pm_flushfence(&t);
  pm_flushfence(t);
  pm_flushfence(&t->root);
  pm_flushfence(t->root);
  pm_flushfence(&n->next);
  pm_flushfence(n->next);

  t->size = 1;
  t->root = n;
  *t->root = *n;
  n->data = 1;
  n->next = n;
  *n->next = *n;
}