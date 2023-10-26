#ifndef BIT_UTIL_H_
#define BIT_UTIL_H_

#if defined(_MSC_VER)
#if defined(_M_AMD64) || defined(_M_X64)
#include <intrin.h> // IWYU pragma: keep
#include <nmmintrin.h>
#endif

#pragma intrinsic(_BitScanReverse)
#pragma intrinsic(_BitScanForward)
#define DEC128_POPCOUNT64 __popcnt64
#define DEC128_POPCOUNT32 __popcnt
#else
#define DEC128_POPCOUNT64 __builtin_popcountll
#define DEC128_POPCOUNT32 __builtin_popcount
#endif

// Returns a mask for the bit_index lower order bits.
// Only valid for bit_index in the range [0, 64).
static inline uint64_t LeastSignificantBitMask(int64_t bit_index) {
  return (((uint64_t)(1)) << bit_index) - 1;
}

/// \brief Count the number of leading zeros in an unsigned integer.
static inline int CountLeadingZeros(uint32_t value) {
#if defined(__clang__) || defined(__GNUC__)
  if (value == 0)
    return 32;
  return ((int)__builtin_clz(value));
#elif defined(_MSC_VER)
  unsigned long index;                                   // NOLINT
  if (_BitScanReverse(&index, (unsigned long)(value))) { // NOLINT
    return 31 - ((int)index);
  } else {
    return 32;
  }
#else
  int bitpos = 0;
  while (value != 0) {
    value >>= 1;
    ++bitpos;
  }
  return 32 - bitpos;
#endif
}

#endif
