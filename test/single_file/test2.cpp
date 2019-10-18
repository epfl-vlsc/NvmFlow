#include "annot.h"

struct node{
  long data;
  dur_field node* next;
};

struct tree{
  node* root;
  long size;
  bool arr[8];
};

void lhs(tree* t){
  node* n = t->root;
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

node* rhs(tree* t){
  node* n = new node;
  t = t;
  t = nullptr;
  *t = *t;
  n = t->root;
  *n = *t->root;
  n->data = t->size;
  t->root = n;
  *t->root = *n;
  n = n->next;
  *n = *n->next;
  return n;
}


node* dptr(node** t){
  node** head = t;
  t = new node*;
  *head = nullptr;
  node* n = *head;
  n = n->next;
  *n = **head;
  return n;
}

node* simp(tree* t){
  node* n = new node;
  *t = *t;
  *t->root = *n;
  *n = *t->root;
  *n = *n->next;
  return n;
}