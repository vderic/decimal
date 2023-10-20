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

  decimal_status_t s;

  int64_t v = -12345678;
  decimal128_t dec = dec128_from_int64(v);
  print_dec128(dec);

  decimal128_t rescale;
  s = dec128_rescale(dec, 3, 5, &rescale);
  if (s != DEC128_STATUS_SUCCESS) {
    fprintf(stderr, "rescale failed\n");
    return 1;
  }

  print_dec128(rescale);

  decimal128_t divisor = dec128_from_int64(2);
  decimal128_t result, remainder;

  dec128_divide(dec, divisor, &result, &remainder);

  print_dec128(result);
  print_dec128(remainder);

  decimal128_t dec2 = dec128_from_hilo(100, 330);
  print_dec128(dec2);

  decimal128_t dec3 = {{100, 300}};
  print_dec128(dec3);
  return 0;
}
