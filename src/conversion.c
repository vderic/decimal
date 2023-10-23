#include "basic_decimal.h"
#include "bit_util.h"
#include "decimal_internal.h"
#include <assert.h>

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
    out += snprintf(out, "%s", str, dotpos);
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
    out += snprintf(out, "%s", str, n);
    out += sprintf(out, ".");
    out += snprintf(out, "%s", str + n);
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
                                    int32_t *precision, int32_t *scale);

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
