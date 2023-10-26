#ifndef MACROS_H_
#define MACROS_H_

#include <stdint.h>

#define DEC128_EXPAND(x) x
#define DEC128_STRINGIFY(x) #x
#define DEC128_CONCAT(x, y) x##y

#define DEC128_UNUSED(x) (void)(x)
#define DEC128_ARG_UNUSED(x)
//
// GCC can be told that a certain branch is not likely to be taken (for
// instance, a CHECK failure), and use that information in static analysis.
// Giving it this information can help it optimize for the common case in
// the absence of better information (ie. -fprofile-arcs).
//
#if defined(__GNUC__)
#define DEC128_PREDICT_FALSE(x) (__builtin_expect(!!(x), 0))
#define DEC128_PREDICT_TRUE(x) (__builtin_expect(!!(x), 1))
#define DEC128_NORETURN __attribute__((noreturn))
#define DEC128_NOINLINE __attribute__((noinline))
#define DEC128_PREFETCH(addr) __builtin_prefetch(addr)
#elif defined(_MSC_VER)
#define DEC128_NORETURN __declspec(noreturn)
#define DEC128_NOINLINE __declspec(noinline)
#define DEC128_PREDICT_FALSE(x) (x)
#define DEC128_PREDICT_TRUE(x) (x)
#define DEC128_PREFETCH(addr)
#else
#define DEC128_NORETURN
#define DEC128_PREDICT_FALSE(x) (x)
#define DEC128_PREDICT_TRUE(x) (x)
#define DEC128_PREFETCH(addr)
#endif

#if defined(__GNUC__) || defined(__clang__) || defined(_MSC_VER)
#define DEC128_RESTRICT __restrict
#else
#define DEC128_RESTRICT
#endif

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#endif
