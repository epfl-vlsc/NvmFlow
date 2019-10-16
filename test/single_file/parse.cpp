#include "annot.h"

struct ob{
  int a;
  int b;
};

struct node{
  long data;
  ob field;
  dur_field node* next;
  node* next2;
};

struct tree{
  node** z;
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
  pm_flushfence(&p);
  pm_flushfence(&t);
  pm_flushfence(t);
  pm_flushfence(&t->root);
  pm_flushfence(t->root);
  pm_flushfence(&t->size);
  pm_flushfence(&t->arr[2]);
  pm_flushfence(&n);
  pm_flushfence(n);
  pm_flushfence(&n->next);
  pm_flushfence(n->next);
  
  *p = new node;
  t = new tree;
  *t = *(new tree);
  t->root = new node;
  *t->root = *(new node);
  t->size = 1;
  t->arr[2] = 1;
  n = new node;
  *n = *(new node);
  n->next = new node;
  *n->next = *(new node);
}

void allrhs(tree*& t, node*& n, tree *&x, node *&y, node **&z){
  node** p = (node**)n;
  
  x->root = t->root;
  *x->root = *t->root;
  x->size = t->size;
  x->arr[1] = t->arr[1];
  *x = *t;
  x = t;
 
  y->next = n->next;
  *y->next = *n->next;
  y->next2 = n->next2;
  *y = *n->next2;
  y->data = n->data;
  y->arr[2] = n->arr[2];
  y = n;
  *y = *n;

  y = *p;
  z = p;
  z = &n;
  n = nullptr;
}