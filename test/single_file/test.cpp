#include "annot.h"

struct Field{
  int a;
  int b;
};

struct node{
  long data;
  Field field;
  dur_field node* next;
};

struct tree{
  node* root;
  long size;
  bool arr[8];
};

void alllhs(tree* t){
  node* n = t->root;
  node** p = &n;
  pm_flushfence(*p);
  pm_flushfence(&(*p)->data);
  pm_flushfence(&(*p)->field);
  pm_flushfence((*p)->next);
  pm_flushfence((*p)->next);
  pm_flushfence(p);
  pm_flushfence(&p);
  pm_flushfence(&t);
  pm_flushfence(t);
  pm_flushfence(&t->root);
  pm_flushfence(t->root);
  pm_flushfence(&t->size);
  pm_flushfence(&n);
  pm_flushfence(n);
  pm_flushfence(&n->next);
  pm_flushfence(n->next);

  t = new tree;
  *t = *(new tree);
  t->root = new node;
  *t->root = *(new node);
  t->size = 1;
  n = new node;
  *n = *(new node);
  n->next = new node;
  *n->next = *(new node);
}