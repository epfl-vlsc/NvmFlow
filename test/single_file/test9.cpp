#include "annot.h"

int main() {
  int a = 5;
  if(cond())
    mutateInt(&a);
  else
    mutateInt(&a);

  mutateInt(&a);
  return 0;
}
