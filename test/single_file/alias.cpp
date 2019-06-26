volatile int z;

struct A {
  int a;
  int b;
};

void m(A& b) {
  A* ptr = &b;
  ptr->a = 5;
  z = ptr->a;
}