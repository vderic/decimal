#pragma once

#include "basic_decimal.h"
#include <stdexcept>

inline bool operator==(const decimal128_t &left, const decimal128_t &right) {
  return dec128_cmpeq(left, right);
}

inline bool operator!=(const decimal128_t &left, const decimal128_t &right) {
  return dec128_cmpne(left, right);
}

inline bool operator<(const decimal128_t &left, const decimal128_t &right) {
  return dec128_cmplt(left, right);
}

inline bool operator<=(const decimal128_t &left, const decimal128_t &right) {
  return dec128_cmple(left, right);
}

inline bool operator>(const decimal128_t &left, const decimal128_t &right) {
  return dec128_cmpgt(left, right);
}
inline bool operator>=(const decimal128_t &left, const decimal128_t &right) {
  return dec128_cmpge(left, right);
}

inline decimal128_t operator-(const decimal128_t &operand) {
  return dec128_negate(operand);
}

inline decimal128_t operator~(const decimal128_t &operand) {
  decimal128_t result =
      dec128_from_hilo(~dec128_high_bits(operand), ~dec128_low_bits(operand));
  return result;
}

inline decimal128_t operator+(const decimal128_t &left,
                              const decimal128_t &right) {
  return dec128_sum(left, right);
}

inline decimal128_t operator-(const decimal128_t &left,
                              const decimal128_t &right) {
  return dec128_subtract(left, right);
}

inline decimal128_t operator*(const decimal128_t &left,
                              const decimal128_t &right) {
  return dec128_multiply(left, right);
}

inline decimal128_t operator/(const decimal128_t &left,
                              const decimal128_t &right) {
  decimal128_t remainder, result;
  decimal_status_t s = dec128_divide(left, right, &result, &remainder);
  if (s != DEC128_STATUS_SUCCESS) {
    throw std::runtime_error("dec128_divide error");
  }
  return result;
}

inline decimal128_t operator%(const decimal128_t &left,
                              const decimal128_t &right) {
  decimal128_t remainder, result;
  decimal_status_t s = dec128_divide(left, right, &result, &remainder);
  if (s != DEC128_STATUS_SUCCESS) {
    throw std::runtime_error("dec128_divide error");
  }
  return remainder;
}
