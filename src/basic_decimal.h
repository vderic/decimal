#ifndef _DECIMAL_H_
#define _DECIMAL_H_

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#define DECIML128_BIT_WIDTH 128
#define BIT_WIDTH 128
#define NWORDS (128/64)

#if LITTLE_ENDIAN
#define HIGHWORDINDEX (NWORDS-1)
#define LOWWORDINDEX 0
#else
#define HIGHWORDINDEX 0
#define LOWWORDINDEX (NWORDS-1)
#endif

typedef struct decimal128_t {
	uint64_t array[NWORDS];
} decimal128_t;

decimal128_t decimal128_from_lowbits(int64_t low_bits);
decimal128_t decimal128_create_from_pointer(const uint8_t* bytes);
decimal128_t decimal128_create_from_hilow(int64_t high, uint64_t low);
decimal128_t decimal128_create_from_int64(int64_t value);
void dec128_to_bytes(decimal128_t *v, uint8_t *out);
int64_t dec128_sign(decimal128_t v);
bool dec128_is_negative(decimal128_t v);
int64_t dec128_high_bits(decimal128_t v);
uint64_t dec128_low_bits(decimal128_t v);

decimal128_t decimal128_from_lowbits(int64_t low_bits) {
	decimal128_t dec = {};
	if (low_bits < 0) {
		for (int i = 0 ; i < NWORDS ; i++) {
			uint64_t zero = 0;
			dec.array[i] = ~zero;
		}
	}
	dec.array[LOWWORDINDEX] = (uint64_t) low_bits;
	return dec;
}

/* Bytes are assumed to be in native-endian byte order */
decimal128_t decimal128_create_from_pointer(const uint8_t* bytes) {
  decimal128_t dec;
  memcpy(&dec, bytes, sizeof(decimal128_t));
  return dec;
}

decimal128_t decimal128_create_from_int64(int64_t value) {
  return decimal128_from_lowbits(value);
}

#if LITTLE_ENDIAN
decimal128_t decimal128_create_from_hilow(int64_t high, uint64_t low) {
  decimal128_t dec;
  dec.array[0] = low;
  dec.array[1] = (uint64_t) high;
  return dec;
}
#else
decimal128_t decimal128_create_from_hilow(int64_t high, uint64_t low) {
  decimal128_t dec;
  dec.array[0] = (uint64_t) high;
  dec.array[1] = low;
  return dec;
}
#endif

decimal128_t *dec128_negate(decimal128_t *v);

decimal128_t *dec128_abs(decimal128_t *v);

void dec128_to_bytes(decimal128_t *v, uint8_t *out) {
	memcpy(out, v->array, NWORDS);
}

// return 1 if positive or zero, -1 if strictly negative
int64_t dec128_sign(decimal128_t v) {
  return 1 | (int64_t)(v.array[HIGHWORDINDEX] >> 63);
}

bool dec128_is_negative(decimal128_t v) {
	return ((int64_t) v.array[HIGHWORDINDEX]) < 0;
}

decimal128_t dec128_sum(decimal128_t left, decimal128_t right);

decimal128_t dec128_substract(decimal128_t left, decimal128_t right);

decimal128_t dec128_multiply(decimal128_t left, decimal128_t right);

decimal128_t dec128_divide(decimal128_t left, decimal128_t right);

decimal128_t *dec128_bitwise_and(decimal128_t *v, uint32_t bits);

decimal128_t *dec128_bitwise_or(decimal128_t *v, uint32_t bits);

decimal128_t dec128_bitwise_shift_left(decimal128_t v, uint32_t bits);

decimal128_t dec128_bitwise_shift_right(decimal128_t v, uint32_t bits);

int64_t dec128_high_bits(decimal128_t v) {
#if LITTLE_ENDIAN
	return (int64_t) v.array[1];
#else 
	return (int64_t) v.array[0];
#endif
}

uint64_t dec128_low_bits(decimal128_t v) {
#if LITTLE_ENDIAN
	return v.array[0];
#else 
	return v.array[1];
#endif
}

void dec128_get_whole_and_fraction(decimal128_t v, int32_t scale, decimal128_t *whole, decimal128_t *fraction);

decimal128_t *dec128_get_scale_multipler(int32_t scale);

decimal128_t *dec128_get_half_scale_multipler(int32_t scale);

int dec128_rescale(decimal128_t v, int32_t original_scale, int32_t new_scale, decimal128_t *out, char errbuf, int errsz);

decimal128_t dec128_increase_scale_by(decimal128_t v, int32_t increase_by);

decimal128_t dec128_reduce_scale_by(decimal128_t v, int32_t reduce_by, bool round);

bool dec128_fits_in_precision(decimal128_t v, int32_t precision);

int32_t dec128_count_leading_binary_zeros(decimal128_t v);


#endif
