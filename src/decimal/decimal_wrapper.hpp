#pragma once

#include "basic_decimal.h"
#include <stdexcept>

/// Represents a signed 128-bit integer in two's complement.
///
/// This class is also compiled into LLVM IR - so, it should not have cpp
/// references like streams and boost.
struct Decimal128 {
  decimal128_t dec;

  /// \brief Empty constructor creates a decimal with a value of 0.
  Decimal128() noexcept { dec = {0}; }

  Decimal128(decimal128_t _dec) noexcept { dec = _dec; }

  Decimal128(int64_t v) noexcept { dec = dec128_from_int64(v); }

  /// \brief Create a Decimal128 from the two's complement representation.
  Decimal128(int64_t hi, uint64_t lo) noexcept { dec = dec128_from_hilo(hi, lo); }

  /// \brief Create a decimal from an array of bytes.
  ///
  /// Bytes are assumed to be in native-endian byte order.
  explicit Decimal128(const uint8_t *bytes) { dec = dec128_from_pointer(bytes); }

  /// \brief Create a decimal from any integer not wider than 64 bits.
  template <typename T,
            typename = typename std::enable_if<
                std::is_integral<T>::value && (sizeof(T) <= sizeof(uint64_t)),
                T>::type>
  constexpr Decimal128(T value) noexcept // NOLINT(runtime/explicit)
  {
    dec = dec128_from_int64(value);
  }

  std::string ToIntegerString() const {
    char ret[DEC128_MAX_STRLEN];
    *ret = 0;
    dec128_to_integer_string(dec, ret);
    return std::string(ret);
  }

  std::string ToString(int32_t scale) const {
    char ret[DEC128_MAX_STRLEN];
    *ret = 0;
    dec128_to_string(dec, ret, scale);
    return std::string(ret);
  }

  float ToFloat(int32_t scale) { return dec128_to_float(dec, scale); }

  double ToDouble(int32_t scale) { return dec128_to_double(dec, scale); }

  static decimal_status_t FromString(const char *s, Decimal128 *out,
                                     int32_t *precision, int32_t *scale) {
    return dec128_from_string(s, &out->dec, precision, scale);
  }

  static decimal_status_t FromString(const std::string &s, Decimal128 *out,
                                     int32_t *precision, int32_t *scale) {
    return dec128_from_string(s.c_str(), &out->dec, precision, scale);
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

  /// \brief Add a number to this one. The result is truncated to 128 bits.
  Decimal128 &operator+=(const Decimal128 &right) {
    dec = dec128_sum(dec, right.dec);
    return *this;
  }

  /// \brief Add a number to this one. The result is truncated to 128 bits.
  Decimal128 &operator-=(const Decimal128 &right) {
    dec = dec128_subtract(dec, right.dec);
    return *this;
  }

  /// \brief Multiply this number by another number. The result is truncated to
  /// 128 bits.
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

  /// \brief Get the high bits of the two's complement representation of the
  /// number.
  int64_t high_bits() const { return dec128_high_bits(dec); }

  /// \brief Get the low bits of the two's complement representation of the
  /// number.
  uint64_t low_bits() const { return dec128_low_bits(dec); }

  /// \brief Convert BasicDecimal128 from one scale to another
  decimal_status_t Rescale(int32_t original_scale, int32_t new_scale,
                           Decimal128 *out) const {
    return dec128_rescale(dec, original_scale, new_scale, &out->dec);
  }

  /// \brief Scale up.
  Decimal128 IncreaseScaleBy(int32_t increase_by) const {
    return Decimal128(dec128_increase_scale_by(dec, increase_by));
  }

  /// \brief Scale down.
  /// - If 'round' is true, the right-most digits are dropped and the result
  /// value is
  ///   rounded up (+1 for +ve, -1 for -ve) based on the value of the dropped
  ///   digits
  ///   (>= 10^reduce_by / 2).
  /// - If 'round' is false, the right-most digits are simply dropped.
  Decimal128 ReduceScaleBy(int32_t reduce_by, bool round = true) const {
    return Decimal128(dec128_reduce_scale_by(dec, reduce_by, round));
  }

  /// Divide this number by right and return the decimal result with scale,
  /// i.e. 12345.6789 / 34.56 = 357.2245052
  /// result precision and scale MUST be calculated by
  /// dec128_DIV_precision_scale
  static Decimal128 Divide(Decimal128 &left, int s1, Decimal128 &right, int s2,
                           int precision, int scale) {
    return {dec128_divide_exact(left.dec, s1, right.dec, s2, precision, scale)};
  }

  /// \brief Absolute value (in-place)
  Decimal128 &Abs() {
    dec = dec128_abs(dec);
    return *this;
  }

  /// \brief Absolute value
  static Decimal128 Abs(const Decimal128 &left) {
    return {dec128_abs(left.dec)};
  }

  /// \brief Negate the current value (in-place)
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
