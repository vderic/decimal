#include "basic_decimal.h"
#include "decimal_internal.h"
#include <math.h>

decimal128_t *dec128_negate(decimal128_t *v) { return 0; }

decimal128_t *dec128_abs(decimal128_t *v) { return 0; }

decimal128_t dec128_sum(decimal128_t left, decimal128_t right) {
  decimal128_t dec = {0};
  return dec;
}

decimal128_t dec128_substract(decimal128_t left, decimal128_t right) {
  decimal128_t dec = {0};
  return dec;
}

decimal128_t dec128_multiply(decimal128_t left, decimal128_t right) {
  decimal128_t dec = {0};
  return dec;
}

decimal128_t dec128_divide(decimal128_t left, decimal128_t right) {
  decimal128_t dec = {0};
  return dec;
}

decimal128_t *dec128_bitwise_and(decimal128_t *v, uint32_t bits) { return 0; }

decimal128_t *dec128_bitwise_or(decimal128_t *v, uint32_t bits) { return 0; }

decimal128_t dec128_bitwise_shift_left(decimal128_t v, uint32_t bits) {
  decimal128_t dec = {0};
  return dec;
}

decimal128_t dec128_bitwise_shift_right(decimal128_t v, uint32_t bits) {
  decimal128_t dec = {0};
  return dec;
}

void dec128_get_whole_and_fraction(decimal128_t v, int32_t scale,
                                   decimal128_t *whole,
                                   decimal128_t *fraction) {
  return;
}

decimal128_t *dec128_get_scale_multipler(int32_t scale) { return 0; }

decimal128_t *dec128_get_half_scale_multipler(int32_t scale) { return 0; }

int dec128_rescale(decimal128_t v, int32_t original_scale, int32_t new_scale,
                   decimal128_t *out, char errbuf, int errsz) {
  return 0;
}

decimal128_t dec128_increase_scale_by(decimal128_t v, int32_t increase_by) {
  decimal128_t dec = {0};
  return dec;
}

decimal128_t dec128_reduce_scale_by(decimal128_t v, int32_t reduce_by,
                                    bool round) {
  decimal128_t dec = {0};
  return dec;
}

bool dec128_fits_in_precision(decimal128_t v, int32_t precision) {
  return false;
}

int32_t dec128_count_leading_binary_zeros(decimal128_t v) { return 0; }
