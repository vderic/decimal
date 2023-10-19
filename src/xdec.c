#include "basic_decimal.h"
#include <stdio.h>
#include <string.h>

void print_dec128(decimal128_t dec) {
  printf("%ld, %ld\n", dec.array[0], dec.array[1]);
}

int main() {

  int64_t v = -123;
  decimal128_t dec = decimal128_from_int64(v);
  print_dec128(dec);

  decimal128_t dec2 = decimal128_from_hilow(100, 330);
  print_dec128(dec2);
  return 0;
}
