#include "basic_decimal.h"
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

/* input */
decimal_status_t dec128_from_string(const char *s, decimal128_t *out,
                                    int32_t *precision, int32_t *scale);

decimal_status_t dec128_from_float(float real, decimal128_t *out,
                                   int32_t precision, int32_t scale);

decimal_status_t dec128_from_double(double real, decimal128_t *out,
                                    int32_t precision, int32_t scale);

/* output to various formats */
decimal_status_t dec128_to_int64(decimal128_t v, int64_t *out);

float dec128_to_float(decimal128_t v, int32_t scale);

double dec128_to_double(decimal128_t v, int32_t scale);

void dec128_to_string(decimal128_t v, char *out, int32_t scale);
