#ifndef INT_UTIL_OVERFLOW_H_
#define INT_UTIL_OVERFLOW_H_

#include <stdint.h>

int64_t SafeSignedAdd(int64_t u, int64_t v);
int64_t SafeSignedSubtract(int64_t u, int64_t v);
int64_t SafeSignedNegate(int64_t u);
int64_t SafeLeftShift(int64_t u, int shift);

/// Signed addition with well-defined behaviour on overflow (as unsigned)
int64_t SafeSignedAdd(int64_t u, int64_t v) {
  return (int64_t)((uint64_t)u + (uint64_t)v);
}

/// Signed subtraction with well-defined behaviour on overflow (as unsigned)
int64_t SafeSignedSubtract(int64_t u, int64_t v) {
  return (int64_t)((uint64_t)u - (uint64_t)v);
}

/// Signed negation with well-defined behaviour on overflow (as unsigned)
int64_t SafeSignedNegate(int64_t u) { return (int64_t)(~((uint64_t)u) + 1); }

/// Signed left shift with well-defined behaviour on negative numbers or
/// overflow
int64_t SafeLeftShift(int64_t u, int shift) {
  return (int64_t)((uint64_t)u) << shift;
}

#endif
