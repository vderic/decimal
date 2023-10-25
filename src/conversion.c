#include "basic_decimal.h"
#include "bit_util.h"
#include "decimal_internal.h"
#include "logging.h"
#include "macros.h"
#include "value_parsing.h"
#include <assert.h>
#include <math.h>

/* Real conversion */

#define POWER_OF_TEN(T, EXP)                                                   \
  {                                                                            \
    const int N = kPrecomputedPowersOfTen;                                     \
    DCHECK(EXP >= -N && exp <= N);                                             \
    return powers_of_ten_##T()[EXP + N];                                       \
  }

#define LARGE_POWER_OF_TEN(T, EXP)                                             \
  {                                                                            \
    const int N = kPrecomputedPowersOfTen;                                     \
    if (ARROW_PREDICT_TRUE(EXP >= -N && exp <= N)) {                           \
      return powers_of_ten_##T()[EXP + N];                                     \
    } else {                                                                   \
      return pow((T)(10), (T)(EXP));                                           \
    }                                                                          \
  }

// Return 10**exp, with a fast lookup, assuming `exp` is within bounds
static float PowerOfTen_float(int32_t exp) { POWER_OF_TEN(float, exp); }

static double PowerOfTen_double(int32_t exp) { POWER_OF_TEN(double, exp); }

// Return 10**exp, with a fast lookup if possible
static float LargePowerOfTen_float(int32_t exp) {
  LARGE_POWER_OF_TEN(float, exp);
}

static double LargePowerOfTen_double(int32_t exp) {
  LARGE_POWER_OF_TEN(double, exp);
}

static decimal128_t DecimalPowerOfTen(int exp) {
  DCHECK(exp >= 0 && exp <= trait_dec128.kMaxPrecision);
  return powers_of_ten_dec128()[exp];
}

// Right shift positive `x` by positive `bits`, rounded half to even
static decimal128_t RoundedRightShift(decimal128_t x, int bits) {
  if (bits == 0) {
    return x;
  }
  int64_t result_hi = dec128_high_bits(x);
  uint64_t result_lo = dec128_low_bits(x);
  uint64_t shifted = 0;
  while (bits >= 64) {
    // Retain the information that set bits were shifted right.
    // This is important to detect an exact half.
    shifted = result_lo | (shifted > 0);
    result_lo = result_hi;
    result_hi >>= 63; // for sign
    bits -= 64;
  }
  if (bits > 0) {
    shifted = (result_lo << (64 - bits)) | (shifted > 0);
    result_lo >>= bits;
    result_lo |= (uint64_t)(result_hi) << (64 - bits);
    result_hi >>= bits;
  }
  // We almost have our result, but now do the rounding.
  const uint64_t kHalf = 0x8000000000000000ULL;
  if (shifted > kHalf) {
    // Strictly more than half => round up
    result_lo += 1;
    result_hi += (result_lo == 0);
  } else if (shifted == kHalf) {
    // Exactly half => round to even
    if ((result_lo & 1) != 0) {
      result_lo += 1;
      result_hi += (result_lo == 0);
    }
  } else {
    // Strictly less than half => round down
  }
  return dec128_from_hilo(result_hi, result_lo);
}

#define FROM_POSITIVE_REAL_APPROX(REAL)                                        \
  {                                                                            \
    /* Approximate algorithm that operates in the FP domain (thus subject      \
     * to precision loss).                                                     \
     */                                                                        \
    const double x = rint(real * PowerOfTen_##REAL(scale));                    \
    const REAL max_abs = PowerOfTen_##REAL(precision);                         \
    if (x <= -max_abs || x >= max_abs) {                                       \
      return DEC128_STATUS_OVERFLOW;                                           \
    }                                                                          \
    /* Extract high and low bits */                                            \
    const double high = floor(ldexp(x, -64));                                  \
    const double low = x - ldexp(high, 64);                                    \
                                                                               \
    DCHECK_GE(high, 0);                                                        \
    DCHECK_LT(high, 9.223372036854776e+18); /* 2**63 */                        \
    DCHECK_GE(low, 0);                                                         \
    DCHECK_LT(low, 1.8446744073709552e+19); /* 2**64 */                        \
    *out = dec128_from_hilo((int64_t)(high), (uint64_t)(low));                 \
    return DEC128_STATUS_SUCCESS;                                              \
  }

static decimal_status_t FromPositiveRealApprox_float(float real,
                                                     int32_t precision,
                                                     int32_t scale,
                                                     decimal128_t *out) {
  FROM_POSITIVE_REAL_APPROX(float);
}

static decimal_status_t FromPositiveRealApprox_double(double real,
                                                      int32_t precision,
                                                      int32_t scale,
                                                      decimal128_t *out) {
  FROM_POSITIVE_REAL_APPROX(double);
}

#define FROM_POSITIVE_REAL(REAL)                                               \
  {                                                                            \
    const int kMantissaBits = trait_##REAL.kMantissaBits;                      \
    const int kMantissaDigits = trait_##REAL.kMantissaDigits;                  \
    const int kMaxPrecision = DEC128_MAX_PRECISION;                            \
                                                                               \
    /* Problem statement: construct the Decimal with the value                 \
     closest to `real * 10^scale`. */                                          \
    if (scale < 0) {                                                           \
      /* Negative scales are not handled below, fall back to approx algorithm  \
       */                                                                      \
      return FromPositiveRealApprox_##REAL(real, precision, scale, out);       \
    }                                                                          \
                                                                               \
    /* 1. Check that `real` is within acceptable bounds. */                    \
    const REAL limit = PowerOfTen_##REAL(precision - scale);                   \
    if (real > limit) {                                                        \
      /* Checking the limit early helps ensure the computations below do not   \
       * overflow.                                                             \
       * NOTE: `limit` is allowed here as rounding can make it smaller than    \
       * the theoretical limit (for example, 1.0e23 < 10^23).                  \
       */                                                                      \
      return DEC128_STATUS_OVERFLOW;                                           \
    }                                                                          \
                                                                               \
    /* 2. Losslessly convert `real` to `mant * 2**k` */                        \
    int binary_exp = 0;                                                        \
    const REAL real_mant = frexp(real, &binary_exp);                           \
    /* `real_mant` is within 0.5 and 1 and has M bits of precision.            \
     * Multiply it by 2^M to get an exact integer.                             \
     */                                                                        \
    const uint64_t mant = (uint64_t)(ldexp(real_mant, kMantissaBits));         \
    const int k = binary_exp - kMantissaBits;                                  \
    /* (note that `real = mant * 2^k`) */                                      \
                                                                               \
    /* 3. Start with `mant`.                                                   \
     * We want to end up with `real * 10^scale` i.e. `mant * 2^k * 10^scale`.  \
     */                                                                        \
    decimal128_t x = dec128_from_int64(mant);                                  \
                                                                               \
    if (k < 0) {                                                               \
      /* k < 0 (i.e. binary_exp < kMantissaBits), is probably the common case  \
       * when converting to decimal. It implies right-shifting by -k bits,     \
       * while multiplying by 10^scale. We also must avoid overflow (losing    \
       * bits on the left) and precision loss (losing bits on the right).      \
       */                                                                      \
      int right_shift_by = -k;                                                 \
      int mul_by_ten_to = scale;                                               \
                                                                               \
      /* At this point, `x` has kMantissaDigits significant digits but it can  \
       * fit kMaxPrecision (excluding sign). We can therefore multiply by up   \
       * to 10^(kMaxPrecision - kMantissaDigits).                              \
       */                                                                      \
      const int kSafeMulByTenTo = kMaxPrecision - kMantissaDigits;             \
                                                                               \
      if (mul_by_ten_to <= kSafeMulByTenTo) {                                  \
        /* Scale is small enough, so we can do it all at once. */              \
        x = dec128_multiply(x, DecimalPowerOfTen(mul_by_ten_to));              \
        x = RoundedRightShift(x, right_shift_by);                              \
      } else {                                                                 \
        /* Scale is too large, we cannot multiply at once without overflow.    \
         * We use an iterative algorithm which alternately shifts left by      \
         * multiplying by a power of ten, and shifts right by a number of      \
         * bits.                                                               \
         */                                                                    \
                                                                               \
        /* First multiply `x` by as large a power of ten as possible           \
         * without overflowing.                                                \
         */                                                                    \
        x = dec128_multiply(x, DecimalPowerOfTen(kSafeMulByTenTo));            \
        mul_by_ten_to -= kSafeMulByTenTo;                                      \
                                                                               \
        /* `x` now has full precision. However, we know we'll only             \
         * keep `precision` digits at the end. Extraneous bits/digits          \
         * on the right can be safely shifted away, before multiplying         \
         * again.                                                              \
         * NOTE: if `precision` is the full precision then the algorithm will  \
         * lose the last digit. If `precision` is almost the full precision,   \
         * there can be an off-by-one error due to rounding.                   \
         */                                                                    \
        const int mul_step = MAX(1, kMaxPrecision - precision);                \
                                                                               \
        /* The running exponent, useful to compute by how much we must         \
         * shift right to make place on the left before the next multiply.     \
         */                                                                    \
        int total_exp = 0;                                                     \
        int total_shift = 0;                                                   \
        while (mul_by_ten_to > 0 && right_shift_by > 0) {                      \
          const int exp = MIN(mul_by_ten_to, mul_step);                        \
          total_exp += exp;                                                    \
          /* The supplementary right shift required so that                    \
           * `x * 10^total_exp / 2^total_shift` fits in the decimal.           \
           */                                                                  \
          DCHECK_LT((size_t)(total_exp), sizeof(kCeilLog2PowersOfTen));        \
          const int bits = MIN(right_shift_by,                                 \
                               kCeilLog2PowersOfTen[total_exp] - total_shift); \
          total_shift += bits;                                                 \
          /* Right shift to make place on the left, then multiply */           \
          x = RoundedRightShift(x, bits);                                      \
          right_shift_by -= bits;                                              \
          /* Should not overflow thanks to the precautions taken */            \
          x = dec128_multiply(x, DecimalPowerOfTen(exp));                      \
          mul_by_ten_to -= exp;                                                \
        }                                                                      \
        if (mul_by_ten_to > 0) {                                               \
          x = dec128_multiply(x, DecimalPowerOfTen(mul_by_ten_to));            \
        }                                                                      \
        if (right_shift_by > 0) {                                              \
          x = RoundedRightShift(x, right_shift_by);                            \
        }                                                                      \
      }                                                                        \
    } else {                                                                   \
      /* k >= 0 implies left-shifting by k bits and multiplying by 10^scale.   \
       * The order of these operations therefore doesn't matter. We know       \
       * we won't overflow because of the limit check above, and we also       \
       * won't lose any significant bits on the right.                         \
       */                                                                      \
      x = dec128_multiply(x, DecimalPowerOfTen(scale));                        \
      x = dec128_bitwise_shift_left(x, k);                                     \
    }                                                                          \
                                                                               \
    /* Rounding might have pushed `x` just above the max precision, check      \
     * again */                                                                \
    if (!dec128_fits_in_precision(x, precision)) {                             \
      return DEC128_STATUS_OVERFLOW;                                           \
    }                                                                          \
    *out = x;                                                                  \
    return DEC128_STATUS_SUCCESS;                                              \
  }

static decimal_status_t FromPositiveReal_float(float real, int32_t precision,
                                               int32_t scale,
                                               decimal128_t *out) {
  FROM_POSITIVE_REAL(float);
}

static decimal_status_t FromPositiveReal_double(double real, int32_t precision,
                                                int32_t scale,
                                                decimal128_t *out) {
  FROM_POSITIVE_REAL(double);
}

#define FROM_REAL(REAL)                                                        \
  {                                                                            \
    DCHECK_GT(precision, 0);                                                   \
    DCHECK_LE(precision, trait_dec128.kMaxPrecision);                          \
    DCHECK_GE(scale, -trait_dec128.kMaxScale);                                 \
    DCHECK_LE(scale, trait_dec128.kMaxScale);                                  \
                                                                               \
    if (!isfinite(x)) {                                                        \
      return DEC128_STATUS_ERROR;                                              \
    }                                                                          \
    if (x == 0) {                                                              \
      *out = dec128_from_int64(0);                                             \
    }                                                                          \
    if (x < 0) {                                                               \
      decimal_status_t s = FromPositiveReal_##REAL(-x, precision, scale, out); \
      if (s != DEC128_STATUS_SUCCESS) {                                        \
        return s;                                                              \
      }                                                                        \
      *out = dec128_negate(*out);                                              \
      return DEC128_STATUS_SUCCESS;                                            \
    } else {                                                                   \
      return FromPositiveReal_##REAL(x, precision, scale, out);                \
    }                                                                          \
  }

decimal_status_t dec128_from_float(float x, decimal128_t *out,
                                   int32_t precision, int32_t scale) {
  FROM_REAL(float);
}
decimal_status_t dec128_from_double(double x, decimal128_t *out,
                                    int32_t precision, int32_t scale) {
  FROM_REAL(double);
}

#define TO_REAL_POSITIVE_NO_SPLIT(REAL)                                        \
  {                                                                            \
    REAL x = two_to_64_##REAL((REAL)(dec128_high_bits(decimal)));              \
    x += (REAL)(dec128_low_bits(decimal));                                     \
    x *= LargePowerOfTen_##REAL(-scale);                                       \
    return x;                                                                  \
  }

static float ToRealPositiveNoSplit_float(decimal128_t decimal, int32_t scale) {
  TO_REAL_POSITIVE_NO_SPLIT(float);
}

static double ToRealPositiveNoSplit_double(decimal128_t decimal,
                                           int32_t scale) {
  TO_REAL_POSITIVE_NO_SPLIT(double);
}

/// An appoximate conversion from Decimal128 to Real that guarantees:
/// 1. If the decimal is an integer, the conversion is exact.
/// 2. If the number of fractional digits is <=
/// RealTraits<Real>::kMantissaDigits (e.g.
///    8 for float and 16 for double), the conversion is within 1 ULP of the
///    exact value.
/// 3. Otherwise, the conversion is within
/// 2^(-RealTraits<Real>::kMantissaDigits+1)
///    (e.g. 2^-23 for float and 2^-52 for double) of the exact value.
/// Here "exact value" means the closest representable value by Real.
#define TO_REAL_POSITIVE(REAL)                                                 \
  {                                                                            \
    if (scale <= 0 ||                                                          \
        (dec128_high_bits(decimal) == 0 &&                                     \
         dec128_low_bits(decimal) <= trait_##REAL.kMaxPreciseInteger)) {       \
      /* No need to split the decimal if it is already an integer (scale <= 0) \
       * or if it can be precisely represented by Real                         \
       */                                                                      \
      return ToRealPositiveNoSplit_##REAL(decimal, scale);                     \
    }                                                                          \
                                                                               \
    /* Split decimal into whole and fractional parts to avoid precision loss   \
     */                                                                        \
    decimal128_t whole_decimal, fraction_decimal;                              \
    dec128_get_whole_and_fraction(decimal, scale, &whole_decimal,              \
                                  &fraction_decimal);                          \
                                                                               \
    REAL whole = ToRealPositiveNoSplit_##REAL(whole_decimal, 0);               \
    REAL fraction = ToRealPositiveNoSplit_##REAL(fraction_decimal, scale);     \
                                                                               \
    return whole + fraction;                                                   \
  }

static float ToRealPositive_float(decimal128_t decimal, int32_t scale) {
  TO_REAL_POSITIVE(float);
}

static double ToRealPositive_double(decimal128_t decimal, int32_t scale) {
  TO_REAL_POSITIVE(double);
}

#define TO_REAL(REAL)                                                          \
  {                                                                            \
    DCHECK_GE(scale, -trait_dec128.kMaxScale);                                 \
    DCHECK_LE(scale, trait_dec128.kMaxScale);                                  \
    if (dec128_is_negative(decimal)) {                                         \
      /* Convert the absolute value to avoid precision loss */                 \
      decimal128_t abs = dec128_negate(decimal);                               \
      return -ToRealPositive_##REAL(abs, scale);                               \
    } else {                                                                   \
      return ToRealPositive_##REAL(decimal, scale);                            \
    }                                                                          \
  }

float dec128_to_float(decimal128_t decimal, int32_t scale) { TO_REAL(float); }

double dec128_to_double(decimal128_t decimal, int32_t scale) {
  TO_REAL(double);
}

/* string conversion */
typedef struct decimal_components_t {
  char whole_digits[DEC128_MAX_STRLEN];
  char fractional_digits[DEC128_MAX_STRLEN];
  int32_t exponent;
  char sign;
  bool has_exponent;
} decimal_components_t;

static inline bool IsSign(char c) { return c == '-' || c == '+'; }

static inline bool IsDot(char c) { return c == '.'; }

static inline bool IsDigit(char c) { return c >= '0' && c <= '9'; }

static inline bool StartsExponent(char c) { return c == 'e' || c == 'E'; }

static inline size_t ParseDigitsRun(const char *s, size_t start, size_t size,
                                    char *out) {
  size_t pos;
  for (pos = start; pos < size; ++pos) {
    if (!IsDigit(s[pos])) {
      break;
    }
  }
  strncpy(out, s + start, pos - start);
  return pos;
}

static bool ParseDecimalComponents(const char *s, decimal_components_t *out) {
  size_t pos = 0;
  size_t size = strlen(s);

  if (size == 0) {
    return false;
  }
  // Sign of the number
  if (IsSign(s[pos])) {
    out->sign = *(s + pos);
    ++pos;
  }
  // First run of digits
  pos = ParseDigitsRun(s, pos, size, (char *)out->whole_digits);
  if (pos == size) {
    return !(*out->whole_digits == 0);
  }
  // Optional dot (if given in fractional form)
  bool has_dot = IsDot(s[pos]);
  if (has_dot) {
    // Second run of digits
    ++pos;
    pos = ParseDigitsRun(s, pos, size, out->fractional_digits);
  }
  if (*out->whole_digits == 0 && *out->fractional_digits == 0) {
    // Need at least some digits (whole or fractional)
    return false;
  }
  if (pos == size) {
    return true;
  }
  // Optional exponent
  if (StartsExponent(s[pos])) {
    ++pos;
    if (pos != size && s[pos] == '+') {
      ++pos;
    }
    out->has_exponent = true;
    return StringToInt32(s + pos, size - pos, &(out->exponent));
  }
  return pos == size;
}

static inline size_t find_first_not_of(char *s, char c) {
  size_t len = strlen(s);
  for (size_t i = 0; i < len; i++) {
    if (s[i] != c) {
      return i;
    }
  }
  return -1;
}

// Iterates over input and for each group of kInt64DecimalDigits multiple out by
// the appropriate power of 10 necessary to add source parsed as uint64 and
// then adds the parsed value of source.
static inline void ShiftAndAdd(const char *input, uint64_t out[],
                               size_t out_size) {
  size_t len = strlen(input);
  for (size_t posn = 0; posn < len;) {
    const size_t group_size = MIN(kInt64DecimalDigits, len - posn);
    const uint64_t multiple = kUInt64PowersOfTen[group_size];
    uint64_t chunk = 0;

    if (!StringToUInt64(input + posn, group_size, &chunk)) {
      // error here
      fprintf(stderr, "parse uint64 error");
      return;
    }

    for (size_t i = 0; i < out_size; ++i) {
      __uint128_t tmp = out[i];
      tmp *= multiple;
      tmp += chunk;
      out[i] = (uint64_t)(tmp & 0xFFFFFFFFFFFFFFFFULL);
      chunk = (uint64_t)(tmp >> 64);
    }
    posn += group_size;
  }
}

static decimal_status_t DecimalFromString(const char *s, decimal128_t *out,
                                          int32_t *precision, int32_t *scale) {
  if (!s || *s == 0) {
    // return Status::Invalid("Empty string cannot be converted to ",
    // type_name);
    return DEC128_STATUS_ERROR;
  }

  decimal_components_t dec = {0};
  if (!ParseDecimalComponents(s, &dec)) {
    // return Status::Invalid("The string '", s, "' is not a valid ", type_name,
    // " number");
    return DEC128_STATUS_ERROR;
  }

  // Count number of significant digits (without leading zeros)
  size_t first_non_zero = find_first_not_of(dec.whole_digits, '0');
  size_t significant_digits = strlen(dec.fractional_digits);
  if (first_non_zero != (size_t)-1) {
    significant_digits += strlen(dec.whole_digits) - first_non_zero;
  }
  int32_t parsed_precision = (int32_t)(significant_digits);

  int32_t parsed_scale = 0;
  if (dec.has_exponent) {
    int32_t adjusted_exponent = dec.exponent;
    parsed_scale = -adjusted_exponent + (int32_t)strlen(dec.fractional_digits);
  } else {
    parsed_scale = (int32_t)strlen(dec.fractional_digits);
  }

  if (out != NULL) {
    size_t N = DEC128_BIT_WIDTH / 64;
    uint64_t little_endian_array[N];
    memset(little_endian_array, 0, sizeof(little_endian_array));
    ShiftAndAdd(dec.whole_digits, little_endian_array, N);
    ShiftAndAdd(dec.fractional_digits, little_endian_array, N);
    *out = dec128_from_hilo((int64_t)little_endian_array[1],
                            little_endian_array[0]);
    if (dec.sign == '-') {
      *out = dec128_negate(*out);
    }
  }

  if (parsed_scale < 0) {
    // Force the scale to zero, to avoid negative scales (due to compatibility
    // issues with external systems such as databases)
    if (-parsed_scale > DEC128_MAX_SCALE) {
      // return Status::Invalid("The string '", s, "' cannot be represented as
      // ", type_name);
      return DEC128_STATUS_ERROR;
    }
    if (out != NULL) {
      decimal128_t multipler = dec128_get_scale_multipler(-parsed_scale);
      *out = dec128_multiply(*out, multipler);
    }
    parsed_precision -= parsed_scale;
    parsed_scale = 0;
  }

  if (precision != NULL) {
    *precision = parsed_precision;
  }
  if (scale != NULL) {
    *scale = parsed_scale;
  }

  return DEC128_STATUS_SUCCESS;
}

/* print */
void dec128_print(FILE *fp, decimal128_t v, int precision, int scale) {
  // DECIMAL: Formula: unscaledValue * 10^(-scale)
  // int32: max precision is 9.
  // int64: max precision is 18.
  // int128: max precision is 38.
  // int256: max precision is 76. (not supported).

  __int128_t value;
  dec128_to_bytes(v, (uint8_t *)&value);

  assert(precision >= 1 && precision <= 38);
  assert(scale >= 0 && scale < precision);
  const int sign = (value < 0);
  __uint128_t tmp = (sign ? -value : value);

  char buffer[128];
  char *p = &buffer[sizeof(buffer) - 1];
  *p = 0;

  for (; scale > 0; scale--, precision--) {
    *--p = '0' + (tmp % 10);
    tmp /= 10;
  }

  if (*p) {
    *--p = '.';
  }

  for (; precision > 0 && tmp; precision--) {
    *--p = '0' + (tmp % 10);
    tmp /= 10;
  }

  if (*p == '.' || *p == 0) {
    *--p = '0';
  }

  if (sign) {
    *--p = '-';
  }
  fprintf(fp, "%s", p);
  fprintf(fp, "\n");
}

static void AppendLittleEndianArrayToString(const uint64_t array[], size_t n,
                                            char *result) {

  size_t most_significant_elem_idx = n;
  for (size_t i = 0; i < n; i++) {
    if (array[i] != 0) {
      most_significant_elem_idx = i;
    }
  }

  if (most_significant_elem_idx == n) {
    strcpy(result, "0");
    return;
  }

  uint64_t copy[n];
  memcpy(copy, array, sizeof(uint64_t) * n);
  const uint32_t k1e9 = 1000000000U;
  const size_t kNumBits = n * 64;
  // Segments will contain the array split into groups that map to decimal
  // digits, in little endian order. Each segment will hold at most 9 decimal
  // digits. For example, if the input represents 9876543210123456789, then
  // segments will be [123456789, 876543210, 9]. The max number of segments
  // needed = ceil(kNumBits * log(2) / log(1e9)) = ceil(kNumBits / 29.897352854)
  // <= ceil(kNumBits / 29).
  uint32_t segments[(kNumBits + 28) / 29];
  size_t num_segments = 0;
  uint64_t *most_significant_elem = &copy[most_significant_elem_idx];
  do {
    // Compute remainder = copy % 1e9 and copy = copy / 1e9.
    uint32_t remainder = 0;
    uint64_t *elem = most_significant_elem;
    do {
      // Compute dividend = (remainder << 32) | *elem  (a virtual 96-bit
      // integer); *elem = dividend / 1e9; remainder = dividend % 1e9.
      uint32_t hi = (uint32_t)(*elem >> 32);
      uint32_t lo = (uint32_t)(*elem & LeastSignificantBitMask(32));
      uint64_t dividend_hi = (((uint64_t)(remainder)) << 32) | hi;
      uint64_t quotient_hi = dividend_hi / k1e9;
      remainder = (uint32_t)(dividend_hi % k1e9);
      uint64_t dividend_lo = (((uint64_t)(remainder)) << 32) | lo;
      uint64_t quotient_lo = dividend_lo / k1e9;
      remainder = (uint32_t)(dividend_lo % k1e9);
      *elem = (quotient_hi << 32) | quotient_lo;
    } while (elem-- != copy);

    segments[num_segments++] = remainder;
  } while (*most_significant_elem != 0 || most_significant_elem-- != copy);

  char *output = result;
  const uint32_t *segment = &segments[num_segments - 1];
  output += sprintf(output, "%u", *segment);

  while (segment != segments) {
    --segment;
    // Right-pad formatted segment such that e.g. 123 is formatted as
    // "000000123".
    output += sprintf(output, "%09u", *segment);
  }
}

static void AdjustIntegerStringWithScale(char *str, int32_t scale,
                                         char *result) {
  if (scale == 0) {
    return;
  }
  DCHECK(str != NULL);
  DCHECK(!(*str == 0));
  const bool is_negative = *str == '-';
  const int32_t is_negative_offset = (int32_t)(is_negative);
  const int32_t len = strlen(str);
  const int32_t num_digits = len - is_negative_offset;
  const int32_t adjusted_exponent = num_digits - 1 - scale;
  char *out = result;

  /// Note that the -6 is taken from the Java BigDecimal documentation.
  if (scale < 0 || adjusted_exponent < -6) {
    // Example 1:
    // Precondition: *str = "123", is_negative_offset = 0, num_digits = 3, scale
    // = -2,
    //               adjusted_exponent = 4
    // After inserting decimal point: *str = "1.23"
    // After appending exponent: *str = "1.23E+4"
    // Example 2:
    // Precondition: *str = "-123", is_negative_offset = 1, num_digits = 3,
    // scale = 9,
    //               adjusted_exponent = -7
    // After inserting decimal point: *str = "-1.23"
    // After appending exponent: *str = "-1.23E-7"

    int32_t dotpos = 1 + is_negative_offset;
    strncpy(out, str, dotpos);
    out += dotpos;
    out += sprintf(out, ".");
    out += sprintf(out, "%s", str + dotpos);
    out += sprintf(out, "E");
    if (adjusted_exponent >= 0) {
      out += sprintf(out, "+");
    }
    out += sprintf(out, "%d", adjusted_exponent);
    return;
  }

  if (num_digits > scale) {
    const size_t n = (size_t)(len - scale);
    // Example 1:
    // Precondition: *str = "123", len = num_digits = 3, scale = 1, n = 2
    // After inserting decimal point: *str = "12.3"
    // Example 2:
    // Precondition: *str = "-123", len = 4, num_digits = 3, scale = 1, n = 3
    // After inserting decimal point: *str = "-12.3"
    memcpy(out, str, n);
    out += n;
    out += sprintf(out, ".");
    out += sprintf(out, "%s", str + n);
    return;
  }

  // Example 1:
  // Precondition: *str = "123", is_negative_offset = 0, num_digits = 3, scale =
  // 4 After insert: *str = "000123" After setting decimal point: *str =
  // "0.0123" Example 2: Precondition: *str = "-123", is_negative_offset = 1,
  // num_digits = 3, scale = 4 After insert: *str = "-000123" After setting
  // decimal point: *str = "-0.0123"
  char *p = out;
  if (is_negative) {
    *p = '-';
    p++;
  }
  for (int i = 0; i < scale - num_digits + 2; i++) {
    *p = '0';
    p++;
  }
  p += sprintf(p, "%s", str + is_negative_offset);
  out[is_negative_offset + 1] = '.';
}

/* input */
decimal_status_t dec128_from_string(const char *s, decimal128_t *out,
                                    int32_t *precision, int32_t *scale) {
  return DecimalFromString(s, out, precision, scale);
}

/* output to various formats */
decimal_status_t dec128_to_integer_string(decimal128_t v, char *out) {
  char *p = out;
  if (dec128_high_bits(v) < 0) {
    *p = '-';
    p++;
    decimal128_t abs = dec128_negate(v);
    uint64_t array[] = {dec128_low_bits(abs), (uint64_t)dec128_high_bits(abs)};
    AppendLittleEndianArrayToString(array, 2, p);
  } else {
    uint64_t array[] = {dec128_low_bits(v), (uint64_t)dec128_high_bits(v)};
    AppendLittleEndianArrayToString(array, 2, p);
  }
  return DEC128_STATUS_SUCCESS;
}

int64_t dec128_to_int64(decimal128_t v) {
  CHECKX((dec128_high_bits(v) == 0 || dec128_high_bits(v) == -1),
         "Try to get a decimal128 greater than the value range of a int64_t; "
         "high bits() must be equal to 0 or -1.");
  return (int64_t)dec128_low_bits(v);
}

void dec128_to_string(decimal128_t v, char *out, int32_t scale) {
  decimal_status_t s;
  char intstr[DEC128_MAX_STRLEN];
  s = dec128_to_integer_string(v, intstr);
  DCHECK_OK(s);
  AdjustIntegerStringWithScale(intstr, scale, out);
}
