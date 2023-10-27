#ifndef _DECIMAL_H_
#define _DECIMAL_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
#define DEC128_EXTERN_BEGIN extern "C" {
#define DEC128_EXTERN_END }
#else
#define DEC128_EXTERN_BEGIN
#define DEC128_EXTERN_END
#endif

DEC128_EXTERN_BEGIN

#define DEC128_MAX_PRECISION 38
#define DEC128_MAX_SCALE 38
#define DEC128_MAX_STRLEN 48

#define DEC128_BIT_WIDTH 128
#define NWORDS (128 / 64)

#if DEC128_LITTLE_ENDIAN
#define HIGHWORDINDEX (NWORDS - 1)
#define LOWWORDINDEX 0
#else
#define HIGHWORDINDEX 0
#define LOWWORDINDEX (NWORDS - 1)
#endif

typedef enum decimal_status_t {
  DEC128_STATUS_SUCCESS,
  DEC128_STATUS_DIVIDEDBYZERO,
  DEC128_STATUS_OVERFLOW,
  DEC128_STATUS_RESCALEDATALOSS,
  DEC128_STATUS_ERROR,
} decimal_status_t;

typedef struct decimal128_t {
  uint64_t array[NWORDS];
} decimal128_t;

/* comparison */
bool dec128_cmpeq(decimal128_t left, decimal128_t right);
bool dec128_cmpne(decimal128_t left, decimal128_t right);
bool dec128_cmplt(decimal128_t left, decimal128_t right);
bool dec128_cmpgt(decimal128_t left, decimal128_t right);
bool dec128_cmpge(decimal128_t left, decimal128_t right);
bool dec128_cmple(decimal128_t left, decimal128_t right);

void dec128_print(FILE *fp, decimal128_t v, int precision, int scale);

/* input from various formats */
static inline decimal128_t dec128_from_lowbits(int64_t low_bits) {
  decimal128_t dec = {};
  if (low_bits < 0) {
    for (int i = 0; i < NWORDS; i++) {
      uint64_t zero = 0;
      dec.array[i] = ~zero;
    }
  }
  dec.array[LOWWORDINDEX] = (uint64_t)low_bits;
  return dec;
}

/* Bytes are assumed to be in native-endian byte order */
static inline decimal128_t dec128_from_pointer(const uint8_t *bytes) {
  decimal128_t dec;
  memcpy(&dec, bytes, sizeof(decimal128_t));
  return dec;
}

static inline decimal128_t dec128_from_int64(int64_t value) {
  return dec128_from_lowbits(value);
}

#if DEC128_LITTLE_ENDIAN
static inline decimal128_t dec128_from_hilo(int64_t high, uint64_t low) {
  decimal128_t dec;
  dec.array[0] = low;
  dec.array[1] = (uint64_t)high;
  return dec;
}
#else
static inline decimal128_t dec128_from_hilo(int64_t high, uint64_t low) {
  decimal128_t dec;
  dec.array[0] = (uint64_t)high;
  dec.array[1] = low;
  return dec;
}
#endif

decimal_status_t dec128_from_string(const char *s, decimal128_t *out,
                                    int32_t *precision, int32_t *scale);

decimal_status_t dec128_from_float(float real, decimal128_t *out,
                                   int32_t precision, int32_t scale);

decimal_status_t dec128_from_double(double real, decimal128_t *out,
                                    int32_t precision, int32_t scale);

/* output to various formats */
static inline void dec128_to_bytes(decimal128_t v, uint8_t *out) {
  memcpy(out, v.array, sizeof(v.array));
}

decimal_status_t dec128_to_integer_string(decimal128_t v, char *out);

int64_t dec128_to_int64(decimal128_t v);

float dec128_to_float(decimal128_t v, int32_t scale);

double dec128_to_double(decimal128_t v, int32_t scale);

void dec128_to_string(decimal128_t v, char *out, int32_t scale);

/* absolute */
decimal128_t *dec128_abs_inplace(decimal128_t *v);
decimal128_t dec128_abs(decimal128_t v);

/* negate */
decimal128_t dec128_negate(decimal128_t v);

// return 1 if positive or zero, -1 if strictly negative
static inline int64_t dec128_sign(decimal128_t v) {
  return 1 | ((int64_t)(v.array[HIGHWORDINDEX]) >> 63);
}

static inline bool dec128_is_negative(decimal128_t v) {
  return ((int64_t)v.array[HIGHWORDINDEX]) < 0;
}

decimal128_t dec128_sum(decimal128_t left, decimal128_t right);

decimal128_t dec128_subtract(decimal128_t left, decimal128_t right);

decimal128_t dec128_multiply(decimal128_t left, decimal128_t right);

decimal_status_t dec128_divide(decimal128_t dividend, decimal128_t divisor,
                               decimal128_t *result, decimal128_t *remainder);

decimal128_t dec128_bitwise_and(decimal128_t left, decimal128_t right);

decimal128_t dec128_bitwise_or(decimal128_t left, decimal128_t right);

decimal128_t dec128_bitwise_shift_left(decimal128_t v, uint32_t bits);

decimal128_t dec128_bitwise_shift_right(decimal128_t v, uint32_t bits);

static inline int64_t dec128_high_bits(decimal128_t v) {
#if DEC128_LITTLE_ENDIAN
  return (int64_t)v.array[1];
#else
  return (int64_t)v.array[0];
#endif
}

static inline uint64_t dec128_low_bits(decimal128_t v) {
#if DEC128_LITTLE_ENDIAN
  return v.array[0];
#else
  return v.array[1];
#endif
}

decimal_status_t dec128_get_whole_and_fraction(decimal128_t v, int32_t scale,
                                               decimal128_t *whole,
                                               decimal128_t *fraction);

decimal128_t dec128_get_scale_multiplier(int32_t scale);

decimal128_t dec128_get_half_scale_multiplier(int32_t scale);

decimal128_t dec128_max_value();

decimal128_t dec128_max(int32_t precision);

decimal_status_t dec128_rescale(decimal128_t v, int32_t original_scale,
                                int32_t new_scale, decimal128_t *out);

decimal128_t dec128_increase_scale_by(decimal128_t v, int32_t increase_by);

decimal128_t dec128_reduce_scale_by(decimal128_t v, int32_t reduce_by,
                                    bool round);

bool dec128_fits_in_precision(decimal128_t v, int32_t precision);

int32_t dec128_count_leading_binary_zeros(decimal128_t v);

/* additional to the original arrow library */
void dec128_ADD_SUB_precision_scale(int p1, int s1, int p2, int s2,
                                    int *precision, int *scale);

void dec128_MUL_precision_scale(int p1, int s1, int p2, int s2, int *precision,
                                int *scale);

void dec128_DIV_precision_scale(int p1, int s1, int p2, int s2, int *precision,
                                int *scale);

void dec128_MOD_precision_scale(int p1, int s1, int p2, int s2, int *precision,
                                int *scale);

// ret_precision and ret_scale must be calculated by dec_DIV_precison_scale
decimal128_t dec128_divide_exact(decimal128_t A, int32_t Ascale, decimal128_t B,
                                 int32_t Bscale, int ret_precision,
                                 int ret_scale);

decimal128_t dec128_floor(decimal128_t A, int scale);

decimal128_t dec128_ceil(decimal128_t A, int scale);

decimal128_t dec128_mod(decimal128_t A, int Ascale, decimal128_t B, int Bscale);

decimal128_t dec128_round(decimal128_t A, int32_t Ascale, int32_t rscale);

DEC128_EXTERN_END

#endif
