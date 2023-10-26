#pragma once

#include "basic_decimal.h"
#include <stdexcept>

struct Decimal128 {
  decimal128_t dec;

  Decimal128() { dec = {0}; }

  Decimal128(decimal128_t _dec) { dec = _dec; }

  Decimal128(int64_t v) { dec = dec128_from_int64(v); }

  Decimal128(int64_t hi, uint64_t lo) { dec = dec128_from_hilo(hi, lo); }

  Decimal128(const uint8_t *bytes) { dec = dec128_from_pointer(bytes); }

  std::string ToString(int32_t scale) {
    char ret[DEC128_MAX_STRLEN];
    dec128_to_string(dec, ret, scale);
    return std::string(ret);
  }

  float ToFloat(int32_t scale) { return dec128_to_float(dec, scale); }

  double ToDouble(int32_t scale) { return dec128_to_double(dec, scale); }

  static decimal_status_t FromString(const char *s, Decimal128 *out,
                                     int32_t *precision, int32_t *scale) {
    return dec128_from_string(s, &out->dec, precision, scale);
  }

  static decimal_status_t FromReal(double real, Decimal128 *out,
                                   int32_t precision, int32_t scale) {
    return dec128_from_double(real, &out->dec, precision, scale);
  }

  static decimal_status_t FromReal(float real, Decimal128 *out,
                                   int32_t precision, int32_t scale) {
    return dec128_from_float(real, &out->dec, precision, scale);
  }

  explicit operator int64_t() const { return dec128_to_int64(dec); }

  Decimal128 &operator+=(const Decimal128 &right) {
    dec = dec128_sum(dec, right.dec);
    return *this;
  }

  Decimal128 &operator-=(const Decimal128 &right) {
    dec = dec128_subtract(dec, right.dec);
    return *this;
  }

  Decimal128 &operator*=(const Decimal128 &right) {
    dec = dec128_multiply(dec, right.dec);
    return *this;
  }

  /* normalize to the same scale before divide */
  Decimal128 &operator/=(const Decimal128 &right) {
    decimal128_t result, remainder;
    decimal_status_t s = dec128_divide(dec, right.dec, &result, &remainder);
    if (s) {
      throw std::runtime_error("dec128_divide failed");
    }
    dec = result;
    return *this;
  }

  /* normalize to the same scale before divide */
  Decimal128 &operator%=(const Decimal128 &right) {
    decimal128_t result, remainder;
    decimal_status_t s = dec128_divide(dec, right.dec, &result, &remainder);
    if (s) {
      throw std::runtime_error("dec128_divide failed");
    }
    dec = remainder;
    return *this;
  }

  int64_t high_bits() { return dec128_high_bits(dec); }

  uint64_t low_bits() { return dec128_low_bits(dec); }

  Decimal128 IncreaseScaleBy(int32_t increase_by) {
    return Decimal128(dec128_increase_scale_by(dec, increase_by));
  }

  Decimal128 ReduceScaleBy(int32_t reduce_by, bool round = true) {
    return Decimal128(dec128_reduce_scale_by(dec, reduce_by, round));
  }

  static Decimal128 Divide(Decimal128 &left, int s1, Decimal128 &right, int s2,
                           int precision, int scale) {
    Decimal128 d1 = left;
    Decimal128 d2 = right;
    if (s1 > s2) {
      d2 = d2.IncreaseScaleBy(s1 - s2);
    } else if (s2 > s1) {
      d1 = d1.IncreaseScaleBy(s2 - s1);
    }

    return {dec128_divide_exact(d1.dec, d2.dec, precision, scale)};
  }

  void print(int precision, int scale) {
    dec128_print(stdout, dec, precision, scale);
  }

  Decimal128 &Abs() {
    dec = dec128_abs(dec);
    return *this;
  }

  static Decimal128 Abs(const Decimal128 &left) {
    return {dec128_abs(left.dec)};
  }

  Decimal128 &Negate() {
    dec = dec128_negate(dec);
    return *this;
  }

  static Decimal128 Ceil(const Decimal128 &left, int32_t scale) {
    return dec128_ceil(left.dec, scale);
  }

  static Decimal128 Floor(const Decimal128 &left, int32_t scale) {
    return dec128_floor(left.dec, scale);
  }

  static Decimal128 Mod(const Decimal128 &left, int32_t left_scale,
                        const Decimal128 &right, int right_scale) {
    return dec128_mod(left.dec, left_scale, right.dec, right_scale);
  }

  static Decimal128 Round(const Decimal128 &left, int32_t scale,
                          int32_t ret_scale) {
    return dec128_round(left.dec, scale, ret_scale);
  }
};

inline decimal_status_t dec128_from_string(const std::string &value,
                                           decimal128_t *out, int *precision,
                                           int *scale) {
  return dec128_from_string(value.c_str(), out, precision, scale);
}

inline bool operator==(const Decimal128 &left, const Decimal128 &right) {
  return dec128_cmpeq(left.dec, right.dec);
}

inline bool operator!=(const Decimal128 &left, const Decimal128 &right) {
  return dec128_cmpne(left.dec, right.dec);
}

inline bool operator<(const Decimal128 &left, const Decimal128 &right) {
  return dec128_cmplt(left.dec, right.dec);
}

inline bool operator<=(const Decimal128 &left, const Decimal128 &right) {
  return dec128_cmple(left.dec, right.dec);
}

inline bool operator>(const Decimal128 &left, const Decimal128 &right) {
  return dec128_cmpgt(left.dec, right.dec);
}
inline bool operator>=(const Decimal128 &left, const Decimal128 &right) {
  return dec128_cmpge(left.dec, right.dec);
}

inline Decimal128 operator-(const Decimal128 &operand) {
  return Decimal128(dec128_negate(operand.dec));
}

inline Decimal128 operator~(const Decimal128 &operand) {
  return Decimal128(~dec128_high_bits(operand.dec),
                    ~dec128_low_bits(operand.dec));
}

inline Decimal128 operator+(const Decimal128 &left, const Decimal128 &right) {
  Decimal128 ret = left;
  ret += right;
  return ret;
}

inline Decimal128 operator-(const Decimal128 &left, const Decimal128 &right) {
  Decimal128 ret = left;
  ret -= right;
  return ret;
}

inline Decimal128 operator*(const Decimal128 &left, const Decimal128 &right) {
  Decimal128 ret = left;
  ret *= right;
  return ret;
}

inline Decimal128 operator/(const Decimal128 &left, const Decimal128 &right) {
  Decimal128 ret = left;
  ret /= right;
  return ret;
}

inline Decimal128 operator%(const Decimal128 &left, const Decimal128 &right) {
  Decimal128 ret = left;
  ret %= right;
  return ret;
}
