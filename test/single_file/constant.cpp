extern void x(int a, int b);

int main() {
  int a, b, c;
  a = 1;
  b = 2;
  if (a) {
    c = a + b;
  }

  x(a, c);

  return 0;
}