#include "basic_decimal.h"
#include "bit_util.h"
#include "decimal_internal.h"
#include "value_parsing.h"
#include <assert.h>
#include <math.h>

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

    char exp[size - pos + 1];
    strncpy(exp, s + pos, size - pos);
    out->exponent = atoi(exp);
    return true;
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

    if (!ParseUInt64(input + posn, group_size, &chunk)) {
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
  if (*s == 0) {
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
  if (first_non_zero != -1) {
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

static void AppendLittleEndianArrayToString(const uint64_t *array, size_t n,
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
  // DCHECK(str != NULL);
  // DCHECK(!(*str == 0));
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
    strncpy(out, str, n);
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

decimal_status_t dec128_from_float(float real, decimal128_t *out,
                                   int32_t precision, int32_t scale);

decimal_status_t dec128_from_double(double real, decimal128_t *out,
                                    int32_t precision, int32_t scale);

/* output to various formats */
decimal_status_t dec128_to_integer_string(decimal128_t v, char *out) {
  char *p = out;
  if (dec128_high_bits(v) < 0) {
    *p = '-';
    p++;
    decimal128_t abs = dec128_negate(v);
    uint64_t array[] = {dec128_low_bits(abs), (uint64_t)dec128_high_bits(abs)};
    AppendLittleEndianArrayToString((uint64_t *)array, 2, p);
  } else {
    uint64_t array[] = {dec128_low_bits(v), (uint64_t)dec128_high_bits(v)};
    AppendLittleEndianArrayToString((uint64_t *)array, 2, p);
  }
  return DEC128_STATUS_SUCCESS;
}

decimal_status_t dec128_to_int64(decimal128_t v, int64_t *out);

float dec128_to_float(decimal128_t v, int32_t scale);

double dec128_to_double(decimal128_t v, int32_t scale);

void dec128_to_string(decimal128_t v, char *out, int32_t scale) {
  decimal_status_t s;
  char intstr[DEC128_MAX_STRLEN];
  s = dec128_to_integer_string(v, intstr);
  AdjustIntegerStringWithScale(intstr, scale, out);
}
