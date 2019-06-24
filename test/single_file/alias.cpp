struct A {
  int* a;
  int* b;

  void __attribute__((noinline)) setA(int* x, int* y) {
    a = x;
    b = y;
  }
};

void set(A* o) {
  o->setA(new int, new int);
  o->a = new int;
}
