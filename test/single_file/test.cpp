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
  int* arr[3];
};

struct tree{
  node** z;
  node* root;
  long size;
  bool arr[8];
};

volatile tree *x;
volatile node *y;
volatile node **z;

void allrhs(tree* t, node* n){
  node** p = (node**)n;
 

  x = t;
  x = nullptr;
  *(tree*)x = *t;
  x->root = t->root;
  *x->root = *t->root;
  x->size = t->size;
  x->arr[1] = t->arr[1];

  y = n;
  *(node*)y = *n;
  y->next = n->next;
  *y->next = *n->next;
  y->next2 = n->next2;
  *y->next2 = *n->next2;
  y->data = n->data;
  y->arr[2] = n->arr[2];

  y = *p;
  z = (volatile node**)p;
  z = (volatile node**)&n;
  
}