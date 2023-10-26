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
    if (DEC128_PREDICT_FALSE(digit > 9U)) {                                    \
      /* Non-digit */                                                          \
      return false;                                                            \
    }                                                                          \
    result = (C_TYPE)(result + digit);                                         \
  } else {                                                                     \
    break;                                                                     \
  }

#define PARSE_UNSIGNED_ITERATION_LAST(C_TYPE, MAX_VALUE)                       \
  if (length > 0) {                                                            \
    if (DEC128_PREDICT_FALSE(result > MAX_VALUE / 10U)) {                      \
      /* Overflow */                                                           \
      return false;                                                            \
    }                                                                          \
    uint8_t digit = ParseDecimalDigit(*s++);                                   \
    result = (C_TYPE)(result * 10U);                                           \
    C_TYPE new_result = (C_TYPE)(result + digit);                              \
    if (DEC128_PREDICT_FALSE(--length > 0)) {                                  \
      /* Too many digits */                                                    \
      return false;                                                            \
    }                                                                          \
    if (DEC128_PREDICT_FALSE(digit > 9U)) {                                    \
      /* Non-digit */                                                          \
      return false;                                                            \
    }                                                                          \
    if (DEC128_PREDICT_FALSE(new_result < result)) {                           \
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

#define PARSEHEX(T)                                                            \
  {                                                                            \
    /* lets make sure that the length of the string is not too big */          \
    if (!DEC128_PREDICT_TRUE(sizeof(T) * 2 >= length && length > 0)) {         \
      return false;                                                            \
    }                                                                          \
    T result = 0;                                                              \
    for (size_t i = 0; i < length; i++) {                                      \
      result = (T)(result << 4);                                               \
      if (s[i] >= '0' && s[i] <= '9') {                                        \
        result = (T)(result | (s[i] - '0'));                                   \
      } else if (s[i] >= 'A' && s[i] <= 'F') {                                 \
        result = (T)(result | (s[i] - 'A' + 10));                              \
      } else if (s[i] >= 'a' && s[i] <= 'f') {                                 \
        result = (T)(result | (s[i] - 'a' + 10));                              \
      } else {                                                                 \
        /* Non-digit */                                                        \
        return false;                                                          \
      }                                                                        \
    }                                                                          \
    *out = result;                                                             \
    return true;                                                               \
  }

static inline bool ParseHexUInt8(const char *s, size_t length, uint8_t *out) {
  PARSEHEX(uint8_t);
}
static inline bool ParseHexUInt16(const char *s, size_t length, uint16_t *out) {
  PARSEHEX(uint16_t);
}
static inline bool ParseHexUInt32(const char *s, size_t length, uint32_t *out) {
  PARSEHEX(uint32_t);
}
static inline bool ParseHexUInt64(const char *s, size_t length, uint64_t *out) {
  PARSEHEX(uint64_t);
}

#define StringToUnsignedInt(T, UTYPE)                                          \
  {                                                                            \
    if (DEC128_PREDICT_FALSE(length == 0)) {                                   \
      return false;                                                            \
    }                                                                          \
    /* If it starts with 0x then its hex */                                    \
    if (length > 2 && s[0] == '0' && ((s[1] == 'x') || (s[1] == 'X'))) {       \
      length -= 2;                                                             \
      s += 2;                                                                  \
                                                                               \
      return DEC128_PREDICT_TRUE(ParseHex##UTYPE(s, length, out));             \
    }                                                                          \
    /* Skip leading zeros */                                                   \
    while (length > 0 && *s == '0') {                                          \
      length--;                                                                \
      s++;                                                                     \
    }                                                                          \
    return Parse##UTYPE(s, length, out);                                       \
  }

static bool StringToUInt8(const char *s, size_t length, uint8_t *out) {
  StringToUnsignedInt(uint8_t, UInt8);
}

static bool StringToUInt16(const char *s, size_t length, uint16_t *out) {
  StringToUnsignedInt(uint16_t, UInt16);
}

static bool StringToUInt32(const char *s, size_t length, uint32_t *out) {
  StringToUnsignedInt(uint32_t, UInt32);
}

static bool StringToUInt64(const char *s, size_t length, uint64_t *out) {
  StringToUnsignedInt(uint64_t, UInt64);
}

#define StringToSignedInt(T, UT, UTYPE, MAX_VALUE)                             \
  {                                                                            \
    UT max_positive = (UT)MAX_VALUE;                                           \
    /* Assuming two's complement */                                            \
    const UT max_negative = max_positive + 1;                                  \
    bool negative = false;                                                     \
    UT unsigned_value = 0;                                                     \
                                                                               \
    if (DEC128_PREDICT_FALSE(length == 0)) {                                   \
      return false;                                                            \
    }                                                                          \
    /* If it starts with 0x then its hex */                                    \
    if (length > 2 && s[0] == '0' && ((s[1] == 'x') || (s[1] == 'X'))) {       \
      length -= 2;                                                             \
      s += 2;                                                                  \
                                                                               \
      if (!DEC128_PREDICT_TRUE(ParseHex##UTYPE(s, length, &unsigned_value))) { \
        return false;                                                          \
      }                                                                        \
      *out = (T)(unsigned_value);                                              \
      return true;                                                             \
    }                                                                          \
                                                                               \
    if (*s == '-') {                                                           \
      negative = true;                                                         \
      s++;                                                                     \
      if (--length == 0) {                                                     \
        return false;                                                          \
      }                                                                        \
    }                                                                          \
    /* Skip leading zeros */                                                   \
    while (length > 0 && *s == '0') {                                          \
      length--;                                                                \
      s++;                                                                     \
    }                                                                          \
    if (!DEC128_PREDICT_TRUE(Parse##UTYPE(s, length, &unsigned_value))) {      \
      return false;                                                            \
    }                                                                          \
    if (negative) {                                                            \
      if (DEC128_PREDICT_FALSE(unsigned_value > max_negative)) {               \
        return false;                                                          \
      }                                                                        \
      /* To avoid both compiler warnings (with unsigned negation)              \
       * and undefined behaviour (with signed negation overflow),              \
       * use the expanded formula for 2's complement negation.                 \
       */                                                                      \
      *out = (T)(~unsigned_value + 1);                                         \
    } else {                                                                   \
      if (DEC128_PREDICT_FALSE(unsigned_value > max_positive)) {               \
        return false;                                                          \
      }                                                                        \
      *out = (T)(unsigned_value);                                              \
    }                                                                          \
    return true;                                                               \
  }

static bool StringToInt8(const char *s, size_t length, int8_t *out) {
  StringToSignedInt(int8_t, uint8_t, UInt8, CHAR_MAX);
}

static bool StringToInt16(const char *s, size_t length, int16_t *out) {
  StringToSignedInt(int16_t, uint16_t, UInt16, SHRT_MAX);
}

static bool StringToInt32(const char *s, size_t length, int32_t *out) {
  StringToSignedInt(int32_t, uint32_t, UInt32, INT_MAX);
}

static bool StringToInt64(const char *s, size_t length, int64_t *out) {
  StringToSignedInt(uint64_t, uint64_t, UInt64, LLONG_MAX);
}

#endif
