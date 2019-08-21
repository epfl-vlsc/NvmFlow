extern void x(int a, int b);

struct A {
  int a;
};

void doStuff(A* obj) {
  int b, c;
  obj->a = 1;
  b = 2;
  if (b) {
    c = obj->a + b;
  }

  x(obj->a, c);
}

int main() {
  doStuff(new A);
  return 0;
}