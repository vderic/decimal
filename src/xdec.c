#include "basic_decimal.h"
#include <stdio.h>
#include <string.h>

int main() {

  int64_t v = 123;
  decimal128_t dec = decimal128_create_from_int64(v);
  printf("%ld, %ld\n", dec.array[0], dec.array[1]);

  return 0;
}
