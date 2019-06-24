struct A;


struct A {
  int* a;
  int b;

  void __attribute__ ((noinline)) setA() {
    a = &b;
    b = 2;
  }
};

void set(A* o) {
  int* x = new int;
  o->setA();
  o->a = x;
}
