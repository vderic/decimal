#include "decimal/basic_decimal.h"
#include "decimal/bit_util.h"
#include "decimal/decimal_internal.h"
#include "decimal/int_util_overflow.h"
#include "decimal/logging.h"
#include "decimal/macros.h"
#include <assert.h>
#include <limits.h>
#include <math.h>

#define NBASE 10000
#define HALF_NBASE 5000
#define DEC_DIGITS 4
#define MUL_GUARD_DIGITS 2
#define DIV_GUARD_DIGITS 4

#if DEC128_LITTLE_ENDIAN
// same as kDecimal128PowersOfTen[38] - 1
static const decimal128_t const_one = {{1, 0}};
static const decimal128_t const_nbase = {{NBASE, 0}};
static const decimal128_t const_half_nbase = {{HALF_NBASE, 0}};
#else
static const decimal128_t const_one = {{0, 1}};
static const decimal128_t const_nbase = {{0, NBASE}};
static const decimal128_t const_half_nbase = {{0, HALF_NBASE}};
#endif

static const decimal128_t const_zero = {0};

void dec128_ADD_SUB_precision_scale(int p1, int s1, int p2, int s2,
                                    int *precision, int *scale) {
  // scale = max(s1, s2);
  // precision = max(p1-s1, p2-s2) + 1 + scale
  // assert(p1 != 0 && p2 != 0);
  *scale = MAX(s1, s2);
  *precision = MAX(p1 - s1, p2 - s2) + 1 + *scale;

#if 0
  CHECKX(*precision <= DEC128_MAX_PRECISION,
         "decimal_ADD: result precision out of range");
#endif
}

void dec128_MUL_precision_scale(int p1, int s1, int p2, int s2, int *precision,
                                int *scale) {
  // scale = s1 + s2
  // precision = precision = p1 + p2 + 1
  // assert(p1 != 0 && p2 != 0);
  *scale = s1 + s2;
  *precision = p1 + p2 + 1;
 #if 0
  CHECKX(*precision <= DEC128_MAX_PRECISION,
         "decimal_MUL: result precision out of range");
 #endif
}

void dec128_DIV_precision_scale(int p1, int s1, int p2, int s2, int *precision,
                                int *scale) {
  // scale = max(4, s1 + p2 - s2 + 1)
  // precision = p1 - s1 + s2 + scale
  // assert(p1 != 0 && p2 != 0);
  *scale = MAX(4, s1 + p2 - s2 + 1);
  *precision = p1 - s1 + s2 + *scale;
 #if 0
  CHECKX(*precision <= DEC128_MAX_PRECISION,
         "decimal_DIV: result precision out of range");
 #endif
}

void dec128_MOD_precision_scale(int p1, int s1, int p2, int s2, int *precision,
                                int *scale) {
  *precision = MAX(p1, p2);
  *scale = MAX(s1, s2);
}

// ret_precision and ret_scale must be calculated by dec_DIV_precison_scale
decimal128_t dec128_divide_exact(decimal128_t A, int32_t Ascale, decimal128_t B,
                                 int32_t Bscale, int ret_precision,
                                 int ret_scale) {

  decimal128_t ret, result, remainder0;

  CHECKX(dec128_cmpne(B, const_zero), "division by zero");

  if (dec128_cmpeq(A, const_zero)) {
    return A;
  }

  if (Ascale > Bscale) {
    B = dec128_increase_scale_by(B, Ascale - Bscale);
  } else if (Bscale > Ascale) {
    A = dec128_increase_scale_by(A, Bscale - Ascale);
  }

  decimal_status_t status = dec128_divide(A, B, &result, &remainder0);
  DCHECK_EQ(status, DEC128_STATUS_SUCCESS);

  int res_ndigits = (ret_scale + DEC_DIGITS - 1) / DEC_DIGITS;
  res_ndigits = MAX(res_ndigits, 1);
  int rscale = res_ndigits * DEC_DIGITS;

  ret = dec128_increase_scale_by(result, rscale);

  for (int i = 0; i < res_ndigits; i++) {
    decimal128_t remainder = remainder0;
    remainder = dec128_multiply(remainder, const_nbase);
    status = dec128_divide(remainder, B, &result, &remainder0);
    CHECKX(status == DEC128_STATUS_SUCCESS, "division failed");

    decimal128_t q = dec128_multiply(
        result,
        dec128_get_scale_multiplier((res_ndigits - i - 1) * DEC_DIGITS));
    ret = dec128_sum(ret, q);
  }

  if (rscale > ret_scale) {
    ret = dec128_reduce_scale_by(ret, rscale - ret_scale, true);
  } else if (rscale < ret_scale) {
    ret = dec128_increase_scale_by(ret, ret_scale - rscale);
  }

  CHECKX(dec128_fits_in_precision(ret, ret_precision),
         "decimal not fit in precision");
  return ret;
}

decimal128_t dec128_floor(decimal128_t A, int scale) {
  decimal128_t whole, fraction, ret;
  dec128_get_whole_and_fraction(A, scale, &whole, &fraction);
  if (dec128_is_negative(whole) && dec128_cmpne(fraction, const_zero)) {
    whole = dec128_subtract(whole, const_one);
  }
  ret = dec128_increase_scale_by(whole, scale);
  return ret;
}

decimal128_t dec128_ceil(decimal128_t A, int scale) {
  decimal128_t whole, fraction, ret;
  dec128_get_whole_and_fraction(A, scale, &whole, &fraction);
  if (!dec128_is_negative(whole) && dec128_cmpne(fraction, const_zero)) {
    whole = dec128_sum(whole, const_one);
  }
  ret = dec128_increase_scale_by(whole, scale);
  return ret;
}

decimal128_t dec128_mod(decimal128_t A, int Ascale, decimal128_t B,
                        int Bscale) {

  decimal_status_t s;
  decimal128_t ret;
  decimal128_t result;
  CHECKX(dec128_cmpne(B, const_zero), "division by zero");

  if (Ascale > Bscale) {
    s = dec128_divide(A, dec128_increase_scale_by(B, Ascale - Bscale), &result,
                      &ret);
  } else if (Ascale < Bscale) {
    s = dec128_divide(dec128_increase_scale_by(A, Bscale - Ascale), B, &result,
                      &ret);
  } else {
    s = dec128_divide(A, B, &result, &ret);
  }
  DCHECK_EQ(s, DEC128_STATUS_SUCCESS);
  return ret;
}

decimal128_t dec128_round(decimal128_t A, int32_t Ascale, int32_t rscale) {
  CHECKX(rscale > 0, "round: invalid scale");

  decimal128_t ret;
  if (Ascale > rscale) {
    int diff = Ascale - rscale;
    ret = dec128_reduce_scale_by(A, diff, true);
    ret = dec128_increase_scale_by(ret, diff);
  } else {
    ret = A;
  }
  return ret;
}
