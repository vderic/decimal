#include "basic_decimal.h"
#include <stdio.h>
#include <string.h>

int main() {

  decimal_status_t s;

  int64_t i64 = -12345678;
  decimal128_t v1 = dec128_from_int64(i64);
  printf("v1\n");
  dec128_print(stdout, v1, 38, 0);

  int64_t hi = -123456;
  uint64_t lo = -22345678;
  decimal128_t v2 = dec128_from_hilo(hi, lo);
  printf("v2\n");
  dec128_print(stdout, v2, 38, 0);

  hi = -123456;
  lo = -2345678;
  decimal128_t v3 = dec128_from_hilo(hi, lo);
  printf("v3\n");
  dec128_print(stdout, v3, 38, 0);

  printf("rescale\n");

  decimal128_t rescale;
  s = dec128_rescale(v1, 3, 5, &rescale);
  if (s != DEC128_STATUS_SUCCESS) {
    fprintf(stderr, "rescale failed\n");
    return 1;
  }

  dec128_print(stdout, rescale, 38, 0);

  printf("divide\n");
  decimal128_t divisor = dec128_from_int64(5);
  decimal128_t result, remainder;

  dec128_divide(v1, divisor, &result, &remainder);

  dec128_print(stdout, result, 38, 0);
  dec128_print(stdout, remainder, 38, 0);

  printf("hilo\n");
  decimal128_t dec2 = dec128_from_hilo(100, 330);
  dec128_print(stdout, dec2, 38, 0);

  decimal128_t dec3 = {{100, 300}};
  dec128_print(stdout, dec3, 38, 0);

  printf("increase scale by\n");
  decimal128_t is = dec128_increase_scale_by(v1, 3);
  dec128_print(stdout, is, 38, 0);

  printf("reduce scale by and round\n");
  decimal128_t rs = dec128_reduce_scale_by(v1, 1, true);
  dec128_print(stdout, rs, 38, 0);

  printf("reduce scale by\n");
  decimal128_t rs2 = dec128_reduce_scale_by(v1, 1, false);
  dec128_print(stdout, rs2, 38, 0);

  printf("sum\n");
  decimal128_t sum = dec128_sum(v1, v2);
  dec128_print(stdout, sum, 38, 0);

  printf("subtract\n");
  decimal128_t minus = dec128_subtract(v3, v2);
  dec128_print(stdout, minus, 38, 0);

  printf("multiply\n");
  decimal128_t multiply = dec128_multiply(v3, dec128_from_int64(2));
  dec128_print(stdout, multiply, 38, 0);

  printf("int128\n");
  __int128_t i128 = -123456778;
  i128 <<= 64;
  i128 -= 6789;
  decimal128_t d128 = dec128_from_pointer((uint8_t *)&i128);
  dec128_print(stdout, d128, 38, 0);

  printf("leading binary zersos\n");
  decimal128_t positive = dec128_from_int64(1234567890);
  int64_t nzeros = dec128_count_leading_binary_zeros(positive);
  printf("nzeros = %ld\n", nzeros);

  printf("fit in precision\n");
  bool fit0 = dec128_fits_in_precision(positive, 4);
  bool fit1 = dec128_fits_in_precision(positive, 11);
  printf("fit0 %d, fit1 %d\n", fit0, fit1);

  printf("fraction\n");
  decimal128_t positive2 = dec128_from_hilo(-1234567890, -9876);
  dec128_print(stdout, positive2, 38, 0);
  decimal128_t whole, fraction;
  s = dec128_get_whole_and_fraction(positive2, 4, &whole, &fraction);
  if (s != DEC128_STATUS_SUCCESS) {
    fprintf(stderr, "whole and fraction failed");
    return 1;
  }
  dec128_print(stdout, whole, 38, 0);
  dec128_print(stdout, fraction, 38, 0);

  char output[DEC128_MAX_STRLEN];
  dec128_to_integer_string(positive2, output);
  printf("dec128_to_integer_string %s\n", output);

  decimal128_t smallint = dec128_from_int64(-123);
  dec128_to_string(smallint, output, 6);
  printf("dec128_to_string %s\n", output);

  int precision, scale;
  decimal128_t from;
  s = dec128_from_string("-12345678901234.9876543", &from, &precision, &scale);

  dec128_print(stdout, from, precision, scale);

  printf("from float\n");
  float f = 123.456;
  precision = 10;
  scale = 4;
  s = dec128_from_float(f, &from, precision, scale);
  if (s != DEC128_STATUS_SUCCESS) {
    fprintf(stderr, "from float error\n");
    return 1;
  }
  dec128_print(stdout, from, precision, scale);
  float f2 = dec128_to_float(from, scale);
  printf("%f %f\n", f, f2);

  printf("from double\n");
  double d = 12345678.456789;
  precision = 16;
  scale = 6;
  s = dec128_from_double(d, &from, precision, scale);
  if (s != DEC128_STATUS_SUCCESS) {
    fprintf(stderr, "from double error\n");
    return 1;
  }
  dec128_print(stdout, from, precision, scale);

  double d2 = dec128_to_double(from, scale);
  printf("%f %f\n", d, d2);

  return 0;
}
