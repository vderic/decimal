#pragma once

#include "basic_decimal.h"

bool operator==(const decimal128_t& left, const decimal128_t &right) {
	return dec128_cmpeq(left, right);
}

bool operator!=(const decimal128_t& left, const decimal128_t &right) {
	return dec128_cmpne(left, right);
}

bool operator < (const decimal128_t& left, const decimal128_t &right) {
	return dec128_cmplt(left, right);
}

bool operator <= (const decimal128_t& left, const decimal128_t &right) {
	return dec128_cmple(left, right);
}
bool operator > (const decimal128_t& left, const decimal128_t &right) {
	return dec128_cmpgt(left, right);
}
bool operator >= (const decimal128_t& left, const decimal128_t &right) {
	return dec128_cmpge(left, right);
}

#if 0
ARROW_EXPORT BasicDecimal128 operator-(const BasicDecimal128& operand);
ARROW_EXPORT BasicDecimal128 operator~(const BasicDecimal128& operand);
ARROW_EXPORT BasicDecimal128 operator+(const BasicDecimal128& left,
                                       const BasicDecimal128& right);
ARROW_EXPORT BasicDecimal128 operator-(const BasicDecimal128& left,
                                       const BasicDecimal128& right);
ARROW_EXPORT BasicDecimal128 operator*(const BasicDecimal128& left,
                                       const BasicDecimal128& right);
ARROW_EXPORT BasicDecimal128 operator/(const BasicDecimal128& left,
                                       const BasicDecimal128& right);
ARROW_EXPORT BasicDecimal128 operator%(const BasicDecimal128& left,
                                       const BasicDecimal128& right);

#endif
