#include "basic_decimal.h"
#include "decimal_internal.h"
#include "int_util_overflow.h"
#include <math.h>

int dec128_compare(decimal128_t v1, decimal128_t v2) { return 0; }
bool dec128_cmpeq(decimal128_t v1, decimal128_t v2) { return false; }
bool dec128_cmplt(decimal128_t v1, decimal128_t v2) { return false; }
bool dec128_cmpgt(decimal128_t v1, decimal128_t v2) { return false; }
bool dec128_cmpge(decimal128_t v1, decimal128_t v2) {
  return !dec128_cmplt(v1, v2);
}
bool dec128_cmple(decimal128_t v1, decimal128_t v2) {
  return !dec128_cmpgt(v1, v2);
}

decimal128_t *dec128_negate(decimal128_t *v) {
  uint64_t result_lo = ~dec128_low_bits(*v) + 1;
  int64_t result_hi = ~dec128_high_bits(*v);
  if (result_lo == 0) {
    result_hi = SafeSignedAdd(result_hi, 1);
  }
  *v = dec128_from_hilo(result_hi, result_lo);
  return v;
}

decimal128_t *dec128_abs_inplace(decimal128_t *v) {
  decimal128_t zero = {};
  if (dec128_cmplt(*v, zero)) {
    dec128_negate(v);
  }
  return v;
}

decimal128_t dec128_abs(decimal128_t v) {
  decimal128_t res = v;
  dec128_abs_inplace(&res);
  return res;
}

decimal128_t dec128_sum(decimal128_t left, decimal128_t right) {
  int64_t result_hi =
      SafeSignedAdd(dec128_high_bits(left), dec128_high_bits(right));
  uint64_t result_lo = dec128_low_bits(left) + dec128_low_bits(right);
  result_hi = SafeSignedAdd(result_hi, result_lo < dec128_low_bits(left));
  return dec128_from_hilo(result_hi, result_lo);
}

decimal128_t dec128_substract(decimal128_t left, decimal128_t right) {
  int64_t result_hi =
      SafeSignedSubtract(dec128_high_bits(left), dec128_high_bits(right));
  uint64_t result_lo = dec128_low_bits(left) - dec128_low_bits(right);
  result_hi = SafeSignedSubtract(result_hi, result_lo > dec128_low_bits(left));
  return dec128_from_hilo(result_hi, result_lo);
}

decimal128_t dec128_multiply(decimal128_t left, decimal128_t right) {
  decimal128_t dec = {0};
  return dec;
}

decimal128_t dec128_divide(decimal128_t left, decimal128_t right) {
  decimal128_t dec = {0};
  return dec;
}

decimal128_t dec128_bitwise_and(decimal128_t left, decimal128_t right) {
  decimal128_t res;
  res.array[0] = left.array[0] & right.array[0];
  res.array[1] = left.array[1] & right.array[1];
  return res;
}

decimal128_t dec128_bitwise_or(decimal128_t left, decimal128_t right) {
  decimal128_t res;
  res.array[0] = left.array[0] | right.array[0];
  res.array[1] = left.array[1] | right.array[1];
  return res;
}

decimal128_t dec128_bitwise_shift_left(decimal128_t v, uint32_t bits) {
  decimal128_t res = v;
  if (bits != 0) {
    uint64_t result_lo;
    int64_t result_hi;
    if (bits < 64) {
      result_hi = SafeLeftShift(dec128_high_bits(v), bits);
      result_hi |= (dec128_low_bits(v) >> (64 - bits));
      result_lo = dec128_low_bits(v) << bits;
    } else if (bits < 128) {
      result_hi = (int64_t)(dec128_low_bits(v) << (bits - 64));
      result_lo = 0;
    } else {
      result_hi = 0;
      result_lo = 0;
    }
    res = dec128_from_hilo(result_hi, result_lo);
  }
  return res;
}

decimal128_t dec128_bitwise_shift_right(decimal128_t v, uint32_t bits) {
  decimal128_t res = v;
  if (bits != 0) {
    uint64_t result_lo;
    int64_t result_hi;
    if (bits < 64) {
      result_lo = dec128_low_bits(v) >> bits;
      result_lo |= (uint64_t)(dec128_high_bits(v)) << (64 - bits);
      result_hi = dec128_high_bits(v) >> bits;
    } else if (bits < 128) {
      result_lo = (uint64_t)(dec128_high_bits(v) >> (bits - 64));
      result_hi = dec128_high_bits(v) >> 63;
    } else {
      result_hi = dec128_high_bits(v) >> 63;
      result_lo = (uint64_t)(result_hi);
    }
    res = dec128_from_hilo(result_hi, result_lo);
  }
  return res;
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
  if (!(precision > 0 && precision < 38)) {
    return false;
  }
  return dec128_cmplt(dec128_abs(v), kDecimal128PowersOfTen[precision]);
}

int32_t dec128_count_leading_binary_zeros(decimal128_t v) { return 0; }
