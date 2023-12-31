#ifndef DECIMAL_INTERNAL_H_
#define DECIMAL_INTERNAL_H_

#include "decimal/basic_decimal.h"
#include <limits.h>
#include <math.h>
#include <stdint.h>

// std::numeric_limits<int64_t>::digits10);
#define kInt64DecimalDigits 18

static const uint64_t kUInt64PowersOfTen[kInt64DecimalDigits + 1] = {
    // clang-format off
    1ULL,
    10ULL,
    100ULL,
    1000ULL,
    10000ULL,
    100000ULL,
    1000000ULL,
    10000000ULL,
    100000000ULL,
    1000000000ULL,
    10000000000ULL,
    100000000000ULL,
    1000000000000ULL,
    10000000000000ULL,
    100000000000000ULL,
    1000000000000000ULL,
    10000000000000000ULL,
    100000000000000000ULL,
    1000000000000000000ULL
    // clang-format on
};

// On the Windows R toolchain, INFINITY is double type instead of float
// constexpr float kFloatInf = std::numeric_limits<float>::infinity();
#define kFloatInf HUGE_VALF

// Attention: these pre-computed constants might not exactly represent their
// decimal counterparts:
//   >>> int(1e38)
//   99999999999999997748809823456034029568

#define kPrecomputedPowersOfTen 76

static const float kFloatPowersOfTen[2 * kPrecomputedPowersOfTen + 1] = {
    0,         0,         0,         0,         0,         0,         0,
    0,         0,         0,         0,         0,         0,         0,
    0,         0,         0,         0,         0,         0,         0,
    0,         0,         0,         0,         0,         0,         0,
    0,         0,         0,         1e-45f,    1e-44f,    1e-43f,    1e-42f,
    1e-41f,    1e-40f,    1e-39f,    1e-38f,    1e-37f,    1e-36f,    1e-35f,
    1e-34f,    1e-33f,    1e-32f,    1e-31f,    1e-30f,    1e-29f,    1e-28f,
    1e-27f,    1e-26f,    1e-25f,    1e-24f,    1e-23f,    1e-22f,    1e-21f,
    1e-20f,    1e-19f,    1e-18f,    1e-17f,    1e-16f,    1e-15f,    1e-14f,
    1e-13f,    1e-12f,    1e-11f,    1e-10f,    1e-9f,     1e-8f,     1e-7f,
    1e-6f,     1e-5f,     1e-4f,     1e-3f,     1e-2f,     1e-1f,     1e0f,
    1e1f,      1e2f,      1e3f,      1e4f,      1e5f,      1e6f,      1e7f,
    1e8f,      1e9f,      1e10f,     1e11f,     1e12f,     1e13f,     1e14f,
    1e15f,     1e16f,     1e17f,     1e18f,     1e19f,     1e20f,     1e21f,
    1e22f,     1e23f,     1e24f,     1e25f,     1e26f,     1e27f,     1e28f,
    1e29f,     1e30f,     1e31f,     1e32f,     1e33f,     1e34f,     1e35f,
    1e36f,     1e37f,     1e38f,     kFloatInf, kFloatInf, kFloatInf, kFloatInf,
    kFloatInf, kFloatInf, kFloatInf, kFloatInf, kFloatInf, kFloatInf, kFloatInf,
    kFloatInf, kFloatInf, kFloatInf, kFloatInf, kFloatInf, kFloatInf, kFloatInf,
    kFloatInf, kFloatInf, kFloatInf, kFloatInf, kFloatInf, kFloatInf, kFloatInf,
    kFloatInf, kFloatInf, kFloatInf, kFloatInf, kFloatInf, kFloatInf, kFloatInf,
    kFloatInf, kFloatInf, kFloatInf, kFloatInf, kFloatInf, kFloatInf};

static const double kDoublePowersOfTen[2 * kPrecomputedPowersOfTen + 1] = {
    1e-76, 1e-75, 1e-74, 1e-73, 1e-72, 1e-71, 1e-70, 1e-69, 1e-68, 1e-67, 1e-66,
    1e-65, 1e-64, 1e-63, 1e-62, 1e-61, 1e-60, 1e-59, 1e-58, 1e-57, 1e-56, 1e-55,
    1e-54, 1e-53, 1e-52, 1e-51, 1e-50, 1e-49, 1e-48, 1e-47, 1e-46, 1e-45, 1e-44,
    1e-43, 1e-42, 1e-41, 1e-40, 1e-39, 1e-38, 1e-37, 1e-36, 1e-35, 1e-34, 1e-33,
    1e-32, 1e-31, 1e-30, 1e-29, 1e-28, 1e-27, 1e-26, 1e-25, 1e-24, 1e-23, 1e-22,
    1e-21, 1e-20, 1e-19, 1e-18, 1e-17, 1e-16, 1e-15, 1e-14, 1e-13, 1e-12, 1e-11,
    1e-10, 1e-9,  1e-8,  1e-7,  1e-6,  1e-5,  1e-4,  1e-3,  1e-2,  1e-1,  1e0,
    1e1,   1e2,   1e3,   1e4,   1e5,   1e6,   1e7,   1e8,   1e9,   1e10,  1e11,
    1e12,  1e13,  1e14,  1e15,  1e16,  1e17,  1e18,  1e19,  1e20,  1e21,  1e22,
    1e23,  1e24,  1e25,  1e26,  1e27,  1e28,  1e29,  1e30,  1e31,  1e32,  1e33,
    1e34,  1e35,  1e36,  1e37,  1e38,  1e39,  1e40,  1e41,  1e42,  1e43,  1e44,
    1e45,  1e46,  1e47,  1e48,  1e49,  1e50,  1e51,  1e52,  1e53,  1e54,  1e55,
    1e56,  1e57,  1e58,  1e59,  1e60,  1e61,  1e62,  1e63,  1e64,  1e65,  1e66,
    1e67,  1e68,  1e69,  1e70,  1e71,  1e72,  1e73,  1e74,  1e75,  1e76};

#if DEC128_LITTLE_ENDIAN
static const decimal128_t kDecimal128PowersOfTen[38 + 1] = {
    {{1LL, 0ULL}},
    {{10LL, 0ULL}},
    {{100LL, 0ULL}},
    {{1000LL, 0ULL}},
    {{10000LL, 0ULL}},
    {{100000LL, 0ULL}},
    {{1000000LL, 0ULL}},
    {{10000000LL, 0ULL}},
    {{100000000LL, 0ULL}},
    {{1000000000LL, 0ULL}},
    {{10000000000LL, 0ULL}},
    {{100000000000LL, 0ULL}},
    {{1000000000000LL, 0ULL}},
    {{10000000000000LL, 0ULL}},
    {{100000000000000LL, 0ULL}},
    {{1000000000000000LL, 0ULL}},
    {{10000000000000000LL, 0ULL}},
    {{100000000000000000LL, 0ULL}},
    {{1000000000000000000LL, 0ULL}},
    {{10000000000000000000ULL, 0LL}},
    {{7766279631452241920ULL, 5LL}},
    {{3875820019684212736ULL, 54LL}},
    {{1864712049423024128ULL, 542LL}},
    {{200376420520689664ULL, 5421LL}},
    {{2003764205206896640ULL, 54210LL}},
    {{1590897978359414784ULL, 542101LL}},
    {{15908979783594147840ULL, 5421010LL}},
    {{11515845246265065472ULL, 54210108LL}},
    {{4477988020393345024ULL, 542101086LL}},
    {{7886392056514347008ULL, 5421010862LL}},
    {{5076944270305263616ULL, 54210108624LL}},
    {{13875954555633532928ULL, 542101086242LL}},
    {{9632337040368467968ULL, 5421010862427LL}},
    {{4089650035136921600ULL, 54210108624275LL}},
    {{4003012203950112768ULL, 542101086242752LL}},
    {{3136633892082024448ULL, 5421010862427522LL}},
    {{12919594847110692864ULL, 54210108624275221LL}},
    {{68739955140067328ULL, 542101086242752217LL}},
    {{687399551400673280ULL, 5421010862427522170LL}}};

static const decimal128_t kDecimal128HalfPowersOfTen[] = {
    {{0ULL, 0ULL}},
    {{5ULL, 0ULL}},
    {{50ULL, 0ULL}},
    {{500ULL, 0ULL}},
    {{5000ULL, 0ULL}},
    {{50000ULL, 0ULL}},
    {{500000ULL, 0ULL}},
    {{5000000ULL, 0ULL}},
    {{50000000ULL, 0ULL}},
    {{500000000ULL, 0ULL}},
    {{5000000000ULL, 0ULL}},
    {{50000000000ULL, 0ULL}},
    {{500000000000ULL, 0ULL}},
    {{5000000000000ULL, 0ULL}},
    {{50000000000000ULL, 0ULL}},
    {{500000000000000ULL, 0ULL}},
    {{5000000000000000ULL, 0ULL}},
    {{50000000000000000ULL, 0ULL}},
    {{500000000000000000ULL, 0ULL}},
    {{5000000000000000000ULL, 0ULL}},
    {{13106511852580896768ULL, 2LL}},
    {{1937910009842106368ULL, 27LL}},
    {{932356024711512064ULL, 271LL}},
    {{9323560247115120640ULL, 2710LL}},
    {{1001882102603448320ULL, 27105LL}},
    {{10018821026034483200ULL, 271050LL}},
    {{7954489891797073920ULL, 2710505LL}},
    {{5757922623132532736ULL, 27105054LL}},
    {{2238994010196672512ULL, 271050543LL}},
    {{3943196028257173504ULL, 2710505431LL}},
    {{2538472135152631808ULL, 27105054312LL}},
    {{6937977277816766464ULL, 271050543121LL}},
    {{14039540557039009792ULL, 2710505431213LL}},
    {{11268197054423236608ULL, 27105054312137LL}},
    {{2001506101975056384ULL, 271050543121376LL}},
    {{1568316946041012224ULL, 2710505431213761LL}},
    {{15683169460410122240ULL, 27105054312137610LL}},
    {{9257742014424809472ULL, 271050543121376108LL}},
    {{343699775700336640ULL, 2710505431213761085LL}}};

#else

static const decimal128_t kDecimal128PowersOfTen[38 + 1] = {
    {{0LL, 1LL}},
    {{0LL, 10LL}},
    {{0LL, 100LL}},
    {{0LL, 1000LL}},
    {{0LL, 10000LL}},
    {{0LL, 100000LL}},
    {{0LL, 1000000LL}},
    {{0LL, 10000000LL}},
    {{0LL, 100000000LL}},
    {{0LL, 1000000000LL}},
    {{0LL, 10000000000LL}},
    {{0LL, 100000000000LL}},
    {{0LL, 1000000000000LL}},
    {{0LL, 10000000000000LL}},
    {{0LL, 100000000000000LL}},
    {{0LL, 1000000000000000LL}},
    {{0LL, 10000000000000000LL}},
    {{0LL, 100000000000000000LL}},
    {{0LL, 1000000000000000000LL}},
    {{0LL, 10000000000000000000ULL}},
    {{5LL, 7766279631452241920ULL}},
    {{54LL, 3875820019684212736ULL}},
    {{542LL, 1864712049423024128ULL}},
    {{5421LL, 200376420520689664ULL}},
    {{54210LL, 2003764205206896640ULL}},
    {{542101LL, 1590897978359414784ULL}},
    {{5421010LL, 15908979783594147840ULL}},
    {{54210108LL, 11515845246265065472ULL}},
    {{542101086LL, 4477988020393345024ULL}},
    {{5421010862LL, 7886392056514347008ULL}},
    {{54210108624LL, 5076944270305263616ULL}},
    {{542101086242LL, 13875954555633532928ULL}},
    {{5421010862427LL, 9632337040368467968ULL}},
    {{54210108624275LL, 4089650035136921600ULL}},
    {{542101086242752LL, 4003012203950112768ULL}},
    {{5421010862427522LL, 3136633892082024448ULL}},
    {{54210108624275221LL, 12919594847110692864ULL}},
    {{542101086242752217LL, 68739955140067328ULL}},
    {{5421010862427522170LL, 687399551400673280ULL}}};

static const decimal128_t kDecimal128HalfPowersOfTen[] = {
    {{0LL, 0ULL}},
    {{0LL, 5ULL}},
    {{0LL, 50ULL}},
    {{0LL, 500ULL}},
    {{0LL, 5000ULL}},
    {{0LL, 50000ULL}},
    {{0LL, 500000ULL}},
    {{0LL, 5000000ULL}},
    {{0LL, 50000000ULL}},
    {{0LL, 500000000ULL}},
    {{0LL, 5000000000ULL}},
    {{0LL, 50000000000ULL}},
    {{0LL, 500000000000ULL}},
    {{0LL, 5000000000000ULL}},
    {{0LL, 50000000000000ULL}},
    {{0LL, 500000000000000ULL}},
    {{0LL, 5000000000000000ULL}},
    {{0LL, 50000000000000000ULL}},
    {{0LL, 500000000000000000ULL}},
    {{0LL, 5000000000000000000ULL}},
    {{2LL, 13106511852580896768ULL}},
    {{27LL, 1937910009842106368ULL}},
    {{271LL, 932356024711512064ULL}},
    {{2710LL, 9323560247115120640ULL}},
    {{27105LL, 1001882102603448320ULL}},
    {{271050LL, 10018821026034483200ULL}},
    {{2710505LL, 7954489891797073920ULL}},
    {{27105054LL, 5757922623132532736ULL}},
    {{271050543LL, 2238994010196672512ULL}},
    {{2710505431LL, 3943196028257173504ULL}},
    {{27105054312LL, 2538472135152631808ULL}},
    {{271050543121LL, 6937977277816766464ULL}},
    {{2710505431213LL, 14039540557039009792ULL}},
    {{27105054312137LL, 11268197054423236608ULL}},
    {{271050543121376LL, 2001506101975056384ULL}},
    {{2710505431213761LL, 1568316946041012224ULL}},
    {{27105054312137610LL, 15683169460410122240ULL}},
    {{271050543121376108LL, 9257742014424809472ULL}},
    {{2710505431213761085LL, 343699775700336640ULL}}};

#endif

// ceil(log2(10 ^ k)) for k in [0...76]
static const int kCeilLog2PowersOfTen[76 + 1] = {
    0,   4,   7,   10,  14,  17,  20,  24,  27,  30,  34,  37,  40,
    44,  47,  50,  54,  57,  60,  64,  67,  70,  74,  77,  80,  84,
    87,  90,  94,  97,  100, 103, 107, 110, 113, 117, 120, 123, 127,
    130, 133, 137, 140, 143, 147, 150, 153, 157, 160, 163, 167, 170,
    173, 177, 180, 183, 187, 190, 193, 196, 200, 203, 206, 210, 213,
    216, 220, 223, 226, 230, 233, 236, 240, 243, 246, 250, 253};

typedef struct RealTrait {
  int kMantissaBits;
  int kMantissaDigits;
  uint64_t kMaxPreciseInteger;
} RealTrait;

/*
template <>
struct RealTraits<float> {
  static constexpr const float* powers_of_ten() { return kFloatPowersOfTen; }

  static constexpr float two_to_64(float x) { return x * 1.8446744e+19f; }
  static constexpr float two_to_128(float x) { return x == 0 ? 0 : kFloatInf; }
  static constexpr float two_to_192(float x) { return x == 0 ? 0 : kFloatInf; }

  static constexpr int kMantissaBits = 24;
  // ceil(log10(2 ^ kMantissaBits))
  static constexpr int kMantissaDigits = 8;
  // Integers between zero and kMaxPreciseInteger can be precisely represented
  static constexpr uint64_t kMaxPreciseInteger = (1ULL << kMantissaBits) - 1;
};
*/
static const RealTrait trait_float = {24, 8, (1ULL << 24) - 1};

static inline const float *powers_of_ten_float() { return kFloatPowersOfTen; }
static inline float two_to_64_float(float x) { return x * 1.8446744e+19f; }
static inline float two_to_128_float(float x) { return x == 0 ? 0 : kFloatInf; }
static inline float two_to_192_float(float x) { return x == 0 ? 0 : kFloatInf; }
/*
template <>
struct RealTraits<double> {
  static constexpr const double* powers_of_ten() { return kDoublePowersOfTen; }

  static constexpr double two_to_64(double x) { return x
* 1.8446744073709552e+19; } static constexpr double two_to_128(double x) {
return x * 3.402823669209385e+38; } static constexpr double two_to_192(double x)
{ return x * 6.277101735386681e+57; }

  static constexpr int kMantissaBits = 53;
  // ceil(log10(2 ^ kMantissaBits))
  static constexpr int kMantissaDigits = 16;
  // Integers between zero and kMaxPreciseInteger can be precisely represented
  static constexpr uint64_t kMaxPreciseInteger = (1ULL << kMantissaBits) - 1;
};
*/
static const RealTrait trait_double = {53, 16, (1ULL << 53) - 1};
static inline const double *powers_of_ten_double() {
  return kDoublePowersOfTen;
}

static inline double two_to_64_double(double x) {
  return x * 1.8446744073709552e+19;
}
static inline double two_to_128_double(double x) {
  return x * 3.402823669209385e+38;
}
static inline double two_to_192_double(double x) {
  return x * 6.277101735386681e+57;
}

typedef struct DecimalTrait {
  int kMaxPrecision;
  int kMaxScale;
} DecimalTrait;

static const DecimalTrait trait_dec128 = {DEC128_MAX_PRECISION,
                                          DEC128_MAX_SCALE};
static inline const decimal128_t *powers_of_ten_dec128() {
  return kDecimal128PowersOfTen;
}

#endif
