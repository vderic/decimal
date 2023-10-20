#include "basic_decimal.h"
#include "bit_util.h"
#include "decimal_internal.h"
#include "int_util_overflow.h"
#include <limits.h>
#include <math.h>

static const uint64_t kInt64Mask = 0xFFFFFFFFFFFFFFFF;

#if LITTLE_ENDIAN
// same as kDecimal128PowersOfTen[38] - 1
static const decimal128_t kMaxDecimal128Value = {
    {687399551400673280ULL - 1, 5421010862427522170LL}};

static const decimal128_t kDecimal128One = {{1, 0}};
#else
// same as kDecimal128PowersOfTen[38] - 1
static const decimal128_t kMaxDecimal128Value = {
    {5421010862427522170LL, 687399551400673280ULL - 1}};

static const decimal128_t kDecimal128One = {{0, 1}};
#endif

static const decimal128_t kDecimal128Zero = {0};

#define UINT128_HIGH_BITS(v) (v >> 64)
#define UINT128_LOW_BITS(v) (v & kInt64Mask)

static inline decimal_status_t DecimalDivide(decimal128_t dividend,
                                             decimal128_t divisor,
                                             decimal128_t *result,
                                             decimal128_t *remainder);

static __uint128_t dec128_to_uint128(decimal128_t v) {
  return (((__uint128_t)dec128_high_bits(v)) << 64) | dec128_low_bits(v);
}

int dec128_compare(decimal128_t v1, decimal128_t v2) { return 0; }

bool dec128_cmpeq(decimal128_t left, decimal128_t right) {
  return dec128_high_bits(left) == dec128_high_bits(right) &&
         dec128_low_bits(left) == dec128_low_bits(right);
}

bool dec128_cmpne(decimal128_t left, decimal128_t right) {
  return !dec128_cmpeq(left, right);
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
  decimal128_t res =
      dec128_from_hilo((int64_t)UINT128_HIGH_BITS(r), UINT128_LOW_BITS(r));
  if (negate) {
    res = dec128_negate(res);
  }
  return res;
}

decimal_status_t dec128_divide(decimal128_t dividend, decimal128_t divisor,
                               decimal128_t *result, decimal128_t *remainder) {
  return DecimalDivide(dividend, divisor, result, remainder);
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

decimal_status_t dec128_get_whole_and_fraction(decimal128_t v, int32_t scale,
                                               decimal128_t *whole,
                                               decimal128_t *fraction) {
  // DCHECK_GE(scale, 0);
  // DCHECK_LE(scale, 38);

  decimal128_t multiplier = kDecimal128PowersOfTen[scale];
  decimal_status_t status = dec128_divide(v, multiplier, whole, fraction);
  // DCHECK_EQ(status, DEC128_STATUS_SUCCESS);
  return status;
}

decimal128_t dec128_get_scale_multipler(int32_t scale) {
  // DCHECK_GE(scale, 0);
  // DCHECK_LE(scale, 38);

  return kDecimal128PowersOfTen[scale];
}

decimal128_t dec128_get_half_scale_multipler(int32_t scale) {
  // DCHECK_GE(scale, 0);
  // DCHECK_LE(scale, 38);

  return kDecimal128HalfPowersOfTen[scale];
}

decimal128_t dec128_max_value() { return kMaxDecimal128Value; }

decimal128_t dec128_max(int32_t precision) {
  // DCHECK_GE(precision, 0);
  // DCHECK_LE(precision, 38);

  return dec128_substract(kDecimal128PowersOfTen[precision],
                          dec128_from_int64(1));
}

static bool rescale_would_cause_data_loss(decimal128_t value,
                                          int32_t delta_scale,
                                          decimal128_t multiplier,
                                          decimal128_t *result) {

  if (delta_scale < 0) {
    // DCHECK_NE(multiplier, 0);
    decimal128_t remainder;
    decimal_status_t status =
        dec128_divide(value, multiplier, result, &remainder);
    // DCHECK_EQ(status, DEC128_STATUS_SUCCESS);
    return dec128_cmpne(remainder, kDecimal128Zero);
  }

  *result = dec128_multiply(value, multiplier);
  return dec128_cmplt(value, kDecimal128Zero) ? dec128_cmpgt(*result, value)
                                              : dec128_cmplt(*result, value);
}

decimal_status_t dec128_rescale(decimal128_t v, int32_t original_scale,
                                int32_t new_scale, decimal128_t *out) {
  // DCHECK_NE(out, NULL);

  if (original_scale == new_scale) {
    *out = v;
    return DEC128_STATUS_SUCCESS;
  }

  const int32_t delta_scale = new_scale - original_scale;
  const int32_t abs_delta_scale = abs(delta_scale);

  decimal128_t multiplier = dec128_get_scale_multipler(abs_delta_scale);

  const bool rescale_data_loss =
      rescale_would_cause_data_loss(v, delta_scale, multiplier, out);

  if (rescale_data_loss) {
    return DEC128_STATUS_RESCALEDATALOSS;
  }
  return DEC128_STATUS_SUCCESS;
}

decimal128_t dec128_increase_scale_by(decimal128_t v, int32_t increase_by) {
  // DCHECK_GE(increase_by 0);
  // DCHECK_LE(increase_by 38);

  return dec128_multiply(v, kDecimal128PowersOfTen[increase_by]);
}

decimal128_t dec128_reduce_scale_by(decimal128_t v, int32_t reduce_by,
                                    bool round) {
  // DCHECK_GE(reduce_by, 0);
  // DCHECK_LLE(reduce_by, 38);

  if (reduce_by == 0) {
    return v;
  }

  decimal128_t divisor = kDecimal128PowersOfTen[reduce_by];
  decimal128_t result;
  decimal128_t remainder;
  decimal_status_t s = dec128_divide(v, divisor, &result, &remainder);
  // DCHECK(s, DEC128_STATUS_SUCCESS);
  if (round) {
    if (dec128_cmpge(dec128_abs(remainder),
                     kDecimal128HalfPowersOfTen[reduce_by])) {
      decimal128_t sign = dec128_from_int64(dec128_sign(v));
      result = dec128_sum(result, sign);
    }
  }
  return result;
}

bool dec128_fits_in_precision(decimal128_t v, int32_t precision) {
  if (!(precision > 0 && precision < 38)) {
    return false;
  }
  return dec128_cmplt(dec128_abs(v), kDecimal128PowersOfTen[precision]);
}

int32_t dec128_count_leading_binary_zeros(decimal128_t v) {
  // DCHECK_GE(v, kDecimal128Zero);

  if (dec128_high_bits(v) == 0) {
    return CountLeadingZeros(dec128_low_bits(v)) + 64;
  } else {
    return CountLeadingZeros((uint64_t)dec128_high_bits(v));
  }
}

/// Expands the given value into a big endian array of ints so that we can work
/// on it. The array will be converted to an absolute value and the was_negative
/// flag will be set appropriately. The array will remove leading zeros from
/// the value.
/// \param array a big endian array of length 4 to set with the value
/// \param was_negative a flag for whether the value was original negative
/// \result the output length of the array
static int64_t FillInArray(decimal128_t value, uint32_t *array,
                           bool *was_negative) {
  decimal128_t abs_value = dec128_abs(value);
  *was_negative = dec128_high_bits(value) < 0;
  uint64_t high = (uint64_t)dec128_high_bits(abs_value);
  uint64_t low = dec128_low_bits(abs_value);

  // FillInArray(std::array<uint64_t, N>& value_array, uint32_t* result_array)
  // is not called here as the following code has better performance, to avoid
  // regression on BasicDecimal128 Division.
  if (high != 0) {
    if (high > UINT_MAX) {
      array[0] = (uint32_t)(high >> 32);
      array[1] = (uint32_t)(high);
      array[2] = (uint32_t)(low >> 32);
      array[3] = (uint32_t)(low);
      return 4;
    }

    array[0] = (uint32_t)(high);
    array[1] = (uint32_t)(low >> 32);
    array[2] = (uint32_t)(low);
    return 3;
  }

  if (low > UINT_MAX) {
    array[0] = (uint32_t)(low >> 32);
    array[1] = (uint32_t)(low);
    return 2;
  }

  if (low == 0) {
    return 0;
  }

  array[0] = (uint32_t)(low);
  return 1;
}

/// Shift the number in the array left by bits positions.
/// \param array the number to shift, must have length elements
/// \param length the number of entries in the array
/// \param bits the number of bits to shift (0 <= bits < 32)
static void ShiftArrayLeft(uint32_t *array, int64_t length, int64_t bits) {
  if (length > 0 && bits != 0) {
    for (int64_t i = 0; i < length - 1; ++i) {
      array[i] = (array[i] << bits) | (array[i + 1] >> (32 - bits));
    }
    array[length - 1] <<= bits;
  }
}

/// Shift the number in the array right by bits positions.
/// \param array the number to shift, must have length elements
/// \param length the number of entries in the array
/// \param bits the number of bits to shift (0 <= bits < 32)
static inline void ShiftArrayRight(uint32_t *array, int64_t length,
                                   int64_t bits) {
  if (length > 0 && bits != 0) {
    for (int64_t i = length - 1; i > 0; --i) {
      array[i] = (array[i] >> bits) | (array[i - 1] << (32 - bits));
    }
    array[0] >>= bits;
  }
}

/// \brief Fix the signs of the result and remainder at the end of the division
/// based on the signs of the dividend and divisor.
static inline void FixDivisionSigns(decimal128_t *result,
                                    decimal128_t *remainder,
                                    bool dividend_was_negative,
                                    bool divisor_was_negative) {
  if (dividend_was_negative != divisor_was_negative) {
    *result = dec128_negate(*result);
  }

  if (dividend_was_negative) {
    *remainder = dec128_negate(*remainder);
  }
}

// \brief Build a native endian array of uint64_t from a big endian array of
// uint32_t.
static decimal_status_t BuildFromArrayToInt64(uint64_t *result_array, int N,
                                              const uint32_t *array,
                                              int64_t length) {
  for (int64_t i = length - 2 * N - 1; i >= 0; i--) {
    if (array[i] != 0) {
      return DEC128_STATUS_OVERFLOW;
    }
  }
  int64_t next_index = length - 1;
  size_t i = 0;
  for (; i < N && next_index >= 0; i++) {
    uint64_t lower_bits = array[next_index--];
    int idx = LITTLE_ENDIAN ? i : N - 1 - i;
    result_array[idx] =
        (next_index < 0)
            ? lower_bits
            : (((uint64_t)(array[next_index--]) << 32) + lower_bits);
  }
  for (; i < N; i++) {
    int idx = LITTLE_ENDIAN ? i : N - 1 - i;
    result_array[idx] = 0;
  }
  return DEC128_STATUS_SUCCESS;
}

/// \brief Build a BasicDecimal128 from a big endian array of uint32_t.
static decimal_status_t BuildFromArray(decimal128_t *value,
                                       const uint32_t *array, int64_t length) {
  uint64_t result_array[NWORDS];
  int idx0 = LITTLE_ENDIAN ? 0 : NWORDS - 1;
  int idx1 = LITTLE_ENDIAN ? 1 : NWORDS - 2;
  decimal_status_t status =
      BuildFromArrayToInt64((uint64_t *)&result_array, NWORDS, array, length);
  if (status != DEC128_STATUS_SUCCESS) {
    return status;
  }
  *value = dec128_from_hilo((int64_t)(result_array[idx1]), result_array[idx0]);
  return DEC128_STATUS_SUCCESS;
}

/// \brief Do a division where the divisor fits into a single 32 bit value.
static inline decimal_status_t
SingleDivide(const uint32_t *dividend, int64_t dividend_length,
             uint32_t divisor, decimal128_t *remainder,
             bool dividend_was_negative, bool divisor_was_negative,
             decimal128_t *result) {
  uint64_t r = 0;
  const int64_t kDecimalArrayLength = DEC128_BIT_WIDTH / sizeof(uint32_t) + 1;
  uint32_t result_array[kDecimalArrayLength];
  for (int64_t j = 0; j < dividend_length; j++) {
    r <<= 32;
    r += dividend[j];
    result_array[j] = (uint32_t)(r / divisor);
    r %= divisor;
  }
  decimal_status_t status =
      BuildFromArray(result, result_array, dividend_length);
  if (status != DEC128_STATUS_SUCCESS) {
    return status;
  }

  *remainder = dec128_from_int64((int64_t)(r));
  FixDivisionSigns(result, remainder, dividend_was_negative,
                   divisor_was_negative);
  return DEC128_STATUS_SUCCESS;
}

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
      FillInArray(dividend, dividend_array + 1, &dividend_was_negative) + 1;
  int64_t divisor_length =
      FillInArray(divisor, divisor_array, &divisor_was_negative);

  // Handle some of the easy cases.
  if (dividend_length <= divisor_length) {
    *remainder = dividend;
    *result = (decimal128_t){0};
    return DEC128_STATUS_SUCCESS;
  }

  if (divisor_length == 0) {
    return DEC128_STATUS_DIVIDEDBYZERO;
  }

  if (divisor_length == 1) {
    return SingleDivide(dividend_array, dividend_length, divisor_array[0],
                        remainder, dividend_was_negative, divisor_was_negative,
                        result);
  }

  int64_t result_length = dividend_length - divisor_length;
  uint32_t result_array[kDecimalArrayLength];
  // DCHECK_LE(result_length, kDecimalArrayLength);

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
    const uint64_t high_dividend =
        ((uint64_t)dividend_array[j]) << 32 | dividend_array[j + 1];
    if (dividend_array[j] != divisor_array[0]) {
      guess = (uint32_t)(high_dividend / divisor_array[0]);
    }

    // catch all of the cases where guess is two too large and most of the
    // cases where it is one too large
    uint32_t rhat =
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
        const uint64_t sum =
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
  decimal_status_t status = BuildFromArray(result, result_array, result_length);
  if (status != DEC128_STATUS_SUCCESS) {
    return status;
  }
  status = BuildFromArray(remainder, dividend_array, dividend_length);
  if (status != DEC128_STATUS_SUCCESS) {
    return status;
  }

  FixDivisionSigns(result, remainder, dividend_was_negative,
                   divisor_was_negative);
  return DEC128_STATUS_SUCCESS;
}
