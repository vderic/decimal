#ifndef VALUE_PARSING_H_
#define VALUE_PARSING_H_

#include "macros.h"
#include <limits.h>
#include <stdint.h>

// NOTE: HalfFloatType would require a half<->float conversion library

static inline uint8_t ParseDecimalDigit(char c) { return (uint8_t)(c - '0'); }

#define PARSE_UNSIGNED_ITERATION(C_TYPE)                                       \
  if (length > 0) {                                                            \
    uint8_t digit = ParseDecimalDigit(*s++);                                   \
    result = (C_TYPE)(result * 10U);                                           \
    length--;                                                                  \
    if (ARROW_PREDICT_FALSE(digit > 9U)) {                                     \
      /* Non-digit */                                                          \
      return false;                                                            \
    }                                                                          \
    result = (C_TYPE)(result + digit);                                         \
  } else {                                                                     \
    break;                                                                     \
  }

#define PARSE_UNSIGNED_ITERATION_LAST(C_TYPE, MAX_VALUE)                       \
  if (length > 0) {                                                            \
    if (ARROW_PREDICT_FALSE(result > MAX_VALUE / 10U)) {                       \
      /* Overflow */                                                           \
      return false;                                                            \
    }                                                                          \
    uint8_t digit = ParseDecimalDigit(*s++);                                   \
    result = (C_TYPE)(result * 10U);                                           \
    C_TYPE new_result = (C_TYPE)(result + digit);                              \
    if (ARROW_PREDICT_FALSE(--length > 0)) {                                   \
      /* Too many digits */                                                    \
      return false;                                                            \
    }                                                                          \
    if (ARROW_PREDICT_FALSE(digit > 9U)) {                                     \
      /* Non-digit */                                                          \
      return false;                                                            \
    }                                                                          \
    if (ARROW_PREDICT_FALSE(new_result < result)) {                            \
      /* Overflow */                                                           \
      return false;                                                            \
    }                                                                          \
    result = new_result;                                                       \
  }

static inline bool ParseUInt8(const char *s, size_t length, uint8_t *out) {
  uint8_t result = 0;

  do {
    PARSE_UNSIGNED_ITERATION(uint8_t);
    PARSE_UNSIGNED_ITERATION(uint8_t);
    PARSE_UNSIGNED_ITERATION_LAST(uint8_t, UCHAR_MAX);
  } while (false);
  *out = result;
  return true;
}

static inline bool ParseUInt16(const char *s, size_t length, uint16_t *out) {
  uint16_t result = 0;
  do {
    PARSE_UNSIGNED_ITERATION(uint16_t);
    PARSE_UNSIGNED_ITERATION(uint16_t);
    PARSE_UNSIGNED_ITERATION(uint16_t);
    PARSE_UNSIGNED_ITERATION(uint16_t);
    PARSE_UNSIGNED_ITERATION_LAST(uint16_t, USHRT_MAX);
  } while (false);
  *out = result;
  return true;
}

static inline bool ParseUInt32(const char *s, size_t length, uint32_t *out) {
  uint32_t result = 0;
  do {
    PARSE_UNSIGNED_ITERATION(uint32_t);
    PARSE_UNSIGNED_ITERATION(uint32_t);
    PARSE_UNSIGNED_ITERATION(uint32_t);
    PARSE_UNSIGNED_ITERATION(uint32_t);
    PARSE_UNSIGNED_ITERATION(uint32_t);

    PARSE_UNSIGNED_ITERATION(uint32_t);
    PARSE_UNSIGNED_ITERATION(uint32_t);
    PARSE_UNSIGNED_ITERATION(uint32_t);
    PARSE_UNSIGNED_ITERATION(uint32_t);

    PARSE_UNSIGNED_ITERATION_LAST(uint32_t, UINT_MAX);
  } while (false);
  *out = result;
  return true;
}

static inline bool ParseUInt64(const char *s, size_t length, uint64_t *out) {
  uint64_t result = 0;
  do {
    PARSE_UNSIGNED_ITERATION(uint64_t);
    PARSE_UNSIGNED_ITERATION(uint64_t);
    PARSE_UNSIGNED_ITERATION(uint64_t);
    PARSE_UNSIGNED_ITERATION(uint64_t);
    PARSE_UNSIGNED_ITERATION(uint64_t);

    PARSE_UNSIGNED_ITERATION(uint64_t);
    PARSE_UNSIGNED_ITERATION(uint64_t);
    PARSE_UNSIGNED_ITERATION(uint64_t);
    PARSE_UNSIGNED_ITERATION(uint64_t);
    PARSE_UNSIGNED_ITERATION(uint64_t);

    PARSE_UNSIGNED_ITERATION(uint64_t);
    PARSE_UNSIGNED_ITERATION(uint64_t);
    PARSE_UNSIGNED_ITERATION(uint64_t);
    PARSE_UNSIGNED_ITERATION(uint64_t);
    PARSE_UNSIGNED_ITERATION(uint64_t);

    PARSE_UNSIGNED_ITERATION(uint64_t);
    PARSE_UNSIGNED_ITERATION(uint64_t);
    PARSE_UNSIGNED_ITERATION(uint64_t);
    PARSE_UNSIGNED_ITERATION(uint64_t);

    PARSE_UNSIGNED_ITERATION_LAST(uint64_t, ULLONG_MAX);
  } while (false);
  *out = result;
  return true;
}

#undef PARSE_UNSIGNED_ITERATION
#undef PARSE_UNSIGNED_ITERATION_LAST

#endif
