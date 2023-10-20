#include "basic_decimal.h"
#include <stdio.h>
#include <string.h>

static void print_dec128(decimal128_t dec) {
#if LITTLE_ENDIAN
  printf("%ld, %ld\n", dec.array[0], dec.array[1]);
#else
  printf("%ld, %ld\n", dec.array[1], dec.array[0]);
#endif
}

int main() {

  int64_t v = -123;
  decimal128_t dec = dec128_from_int64(v);
  print_dec128(dec);

  decimal128_t dec2 = dec128_from_hilo(100, 330);
  print_dec128(dec2);

  decimal128_t dec3 = {{100, 300}};
  print_dec128(dec3);
  return 0;
}
