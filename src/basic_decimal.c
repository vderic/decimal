#include "basic_decimal.h"
#include "bit_util.h"
#include "decimal_internal.h"
#include "int_util_overflow.h"
#include <limits.h>
#include <math.h>

static const uint64_t kInt64Mask = 0xFFFFFFFFFFFFFFFF;

static __uint128_t dec128_to_uint128(decimal128_t v) {
  return (((__uint128_t)dec128_high_bits(v)) << 64) | dec128_low_bits(v);
}

int dec128_compare(decimal128_t v1, decimal128_t v2) { return 0; }

bool dec128_cmpeq(decimal128_t left, decimal128_t right) {
  return dec128_high_bits(left) == dec128_high_bits(right) &&
         dec128_low_bits(left) == dec128_high_bits(right);
}

bool dec128_cmplt(decimal128_t left, decimal128_t right) {
  return dec128_high_bits(left) < dec128_high_bits(right) ||
         (dec128_high_bits(left) == dec128_high_bits(right) &&
          dec128_low_bits(left) < dec128_low_bits(right));
}

bool dec128_cmpgt(decimal128_t left, decimal128_t right) {
  return dec128_cmplt(right, left);
}

bool dec128_cmpge(decimal128_t left, decimal128_t right) {
  return !dec128_cmplt(left, right);
}

bool dec128_cmple(decimal128_t left, decimal128_t right) {
  return !dec128_cmpgt(left, right);
}

decimal128_t dec128_negate(decimal128_t v) {
  uint64_t result_lo = ~dec128_low_bits(v) + 1;
  int64_t result_hi = ~dec128_high_bits(v);
  if (result_lo == 0) {
    result_hi = SafeSignedAdd(result_hi, 1);
  }
  decimal128_t res = dec128_from_hilo(result_hi, result_lo);
  return res;
}

decimal128_t *dec128_abs_inplace(decimal128_t *v) {
  decimal128_t zero = {};
  if (dec128_cmplt(*v, zero)) {
    *v = dec128_negate(*v);
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
  const bool negate = dec128_sign(left) != dec128_sign(right);
  decimal128_t x = dec128_abs(left);
  decimal128_t y = dec128_abs(right);
  __uint128_t r = dec128_to_uint128(x);
  r *= dec128_to_uint128(y);
  decimal128_t res = dec128_from_pointer((uint8_t *)&r);
  if (negate) {
    res = dec128_negate(res);
  }
  return res;
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

static inline decimal_status_t DecimalDivide(decimal128_t dividend,
                                             decimal128_t divisor,
                                             decimal128_t *result,
                                             decimal128_t *remainder) {
  const int64_t kDecimalArrayLength = DEC128_BIT_WIDTH / sizeof(uint32_t);
  // Split the dividend and divisor into integer pieces so that we can
  // work on them.
  uint32_t dividend_array[kDecimalArrayLength + 1];
  uint32_t divisor_array[kDecimalArrayLength];
  bool dividend_was_negative;
  bool divisor_was_negative;
  // leave an extra zero before the dividend
  dividend_array[0] = 0;
  int64_t dividend_length =
      FillInArray(dividend, dividend_array + 1, dividend_was_negative) + 1;
  int64_t divisor_length =
      FillInArray(divisor, divisor_array, divisor_was_negative);

  // Handle some of the easy cases.
  if (dividend_length <= divisor_length) {
    *remainder = dividend;
    *result = (decimal128_t){0};
    return DEC128_SUCCESS;
  }

  if (divisor_length == 0) {
    return DEC128_DIVIDBYZERO;
  }

  if (divisor_length == 1) {
    return SingleDivide(dividend_array, dividend_length, divisor_array[0],
                        remainder, dividend_was_negative, divisor_was_negative,
                        result);
  }

  int64_t result_length = dividend_length - divisor_length;
  uint32_t result_array[kDecimalArrayLength];
  DCHECK_LE(result_length, kDecimalArrayLength);

  // Normalize by shifting both by a multiple of 2 so that
  // the digit guessing is better. The requirement is that
  // divisor_array[0] is greater than 2**31.
  int64_t normalize_bits = CountLeadingZeros(divisor_array[0]);
  ShiftArrayLeft(divisor_array, divisor_length, normalize_bits);
  ShiftArrayLeft(dividend_array, dividend_length, normalize_bits);

  // compute each digit in the result
  for (int64_t j = 0; j < result_length; ++j) {
    // Guess the next digit. At worst it is two too large
    uint32_t guess = UINT_MAX;
    const auto high_dividend =
        ((uint64_t)dividend_array[j]) << 32 | dividend_array[j + 1];
    if (dividend_array[j] != divisor_array[0]) {
      guess = (uint32_t)(high_dividend / divisor_array[0]);
    }

    // catch all of the cases where guess is two too large and most of the
    // cases where it is one too large
    auto rhat =
        (uint32_t)(high_dividend - guess * ((uint64_t)divisor_array[0]));
    while (((uint64_t)divisor_array[1]) * guess >
           (((uint64_t)rhat) << 32) + dividend_array[j + 2]) {
      --guess;
      rhat += divisor_array[0];
      if (((uint64_t)rhat) < divisor_array[0]) {
        break;
      }
    }

    // subtract off the guess * divisor from the dividend
    uint64_t mult = 0;
    for (int64_t i = divisor_length - 1; i >= 0; --i) {
      mult += ((uint64_t)guess) * divisor_array[i];
      uint32_t prev = dividend_array[j + i + 1];
      dividend_array[j + i + 1] -= ((uint32_t)mult);
      mult >>= 32;
      if (dividend_array[j + i + 1] > prev) {
        ++mult;
      }
    }
    uint32_t prev = dividend_array[j];
    dividend_array[j] -= ((uint32_t)mult);

    // if guess was too big, we add back divisor
    if (dividend_array[j] > prev) {
      --guess;
      uint32_t carry = 0;
      for (int64_t i = divisor_length - 1; i >= 0; --i) {
        const auto sum =
            ((uint64_t)divisor_array[i]) + dividend_array[j + i + 1] + carry;
        dividend_array[j + i + 1] = ((uint32_t)sum);
        carry = (uint32_t)(sum >> 32);
      }
      dividend_array[j] += carry;
    }

    result_array[j] = guess;
  }

  // denormalize the remainder
  ShiftArrayRight(dividend_array, dividend_length, normalize_bits);

  // return result and remainder
  auto status = BuildFromArray(result, result_array, result_length);
  if (status != DEC128_SUCCESS) {
    return status;
  }
  status = BuildFromArray(remainder, dividend_array, dividend_length);
  if (status != DEC128_SUCCESS) {
    return status;
  }

  FixDivisionSigns(result, remainder, dividend_was_negative,
                   divisor_was_negative);
  return DEC128_SUCCESS;
}
