#include <stdio.h>
#include "mtwister.h"

int main() {
  MTRand r = seedRand(1337);
  int i;
  for(i=0; i<1000; i++) {
    printf("%f\n", genRand(&r));
  }
  return 0;
}