#include "annot.h"
#include <cstdlib>

int __attribute__((noinline)) num() { return rand(); }

int skip_fnc plus() { return num() + num(); }

int nvm_fnc minus() { return num() - num(); }

int rec() { return rec(); }

int second() { return plus(); }

int first() { return second(); }