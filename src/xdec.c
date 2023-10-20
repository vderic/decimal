#include "basic_decimal.h"
#include <stdio.h>
#include <string.h>

int main() {

  decimal_status_t s;

  int64_t i64 = -12345678;
  decimal128_t v1 = dec128_from_int64(i64);
  printf("v1\n");
  dec128_print(v1);

  int64_t hi = -123456;
  uint64_t lo = -22345678;
  decimal128_t v2 = dec128_from_hilo(hi, lo);
  printf("v2\n");
  dec128_print(v2);

  hi = -123456;
  lo = -2345678;
  decimal128_t v3 = dec128_from_hilo(hi, lo);
  printf("v3\n");
  dec128_print(v3);

  printf("rescale\n");

  decimal128_t rescale;
  s = dec128_rescale(v1, 3, 5, &rescale);
  if (s != DEC128_STATUS_SUCCESS) {
    fprintf(stderr, "rescale failed\n");
    return 1;
  }

  dec128_print(rescale);

  printf("divide\n");
  decimal128_t divisor = dec128_from_int64(5);
  decimal128_t result, remainder;

  dec128_divide(v1, divisor, &result, &remainder);

  dec128_print(result);
  dec128_print(remainder);

  printf("hilo\n");
  decimal128_t dec2 = dec128_from_hilo(100, 330);
  dec128_print(dec2);

  decimal128_t dec3 = {{100, 300}};
  dec128_print(dec3);

  printf("increase scale by\n");
  decimal128_t is = dec128_increase_scale_by(v1, 3);
  dec128_print(is);

  printf("reduce scale by and round\n");
  decimal128_t rs = dec128_reduce_scale_by(v1, 1, true);
  dec128_print(rs);

  printf("reduce scale by\n");
  decimal128_t rs2 = dec128_reduce_scale_by(v1, 1, false);
  dec128_print(rs2);

  printf("sum\n");
  decimal128_t sum = dec128_sum(v1, v2);
  dec128_print(sum);

  printf("subtract\n");
  decimal128_t minus = dec128_subtract(v3, v2);
  dec128_print(minus);

  printf("multiply\n");
  decimal128_t multiply = dec128_multiply(v3, dec128_from_int64(2));
  dec128_print(multiply);

  printf("int128\n");
  __int128_t i128 = -123456778;
  i128 <<= 64;
  i128 -= 6789;
  decimal128_t d128 = dec128_from_pointer((uint8_t *)&i128);
  dec128_print(d128);

  return 0;
}
