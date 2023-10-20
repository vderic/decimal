#include "basic_decimal.h"
#include <stdio.h>
#include <string.h>

int main() {

  decimal_status_t s;

  int64_t v = -12345678;
  decimal128_t dec = dec128_from_int64(v);
  dec128_print(dec);

  printf("rescale\n");

  decimal128_t rescale;
  s = dec128_rescale(dec, 3, 5, &rescale);
  if (s != DEC128_STATUS_SUCCESS) {
    fprintf(stderr, "rescale failed\n");
    return 1;
  }

  dec128_print(rescale);

  printf("divide\n");
  decimal128_t divisor = dec128_from_int64(2);
  decimal128_t result, remainder;

  dec128_divide(dec, divisor, &result, &remainder);

  dec128_print(result);
  dec128_print(remainder);

  printf("hilo\n");
  decimal128_t dec2 = dec128_from_hilo(100, 330);
  dec128_print(dec2);

  decimal128_t dec3 = {{100, 300}};
  dec128_print(dec3);
  return 0;
}
