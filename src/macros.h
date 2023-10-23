#ifndef MACROS_H_
#define MACROS_H_

#include <stdint.h>

#define ARROW_EXPAND(x) x
#define ARROW_STRINGIFY(x) #x
#define ARROW_CONCAT(x, y) x##y

#define ARROW_UNUSED(x) (void)(x)
#define ARROW_ARG_UNUSED(x)
//
// GCC can be told that a certain branch is not likely to be taken (for
// instance, a CHECK failure), and use that information in static analysis.
// Giving it this information can help it optimize for the common case in
// the absence of better information (ie. -fprofile-arcs).
//
#if defined(__GNUC__)
#define ARROW_PREDICT_FALSE(x) (__builtin_expect(!!(x), 0))
#define ARROW_PREDICT_TRUE(x) (__builtin_expect(!!(x), 1))
#define ARROW_NORETURN __attribute__((noreturn))
#define ARROW_NOINLINE __attribute__((noinline))
#define ARROW_PREFETCH(addr) __builtin_prefetch(addr)
#elif defined(_MSC_VER)
#define ARROW_NORETURN __declspec(noreturn)
#define ARROW_NOINLINE __declspec(noinline)
#define ARROW_PREDICT_FALSE(x) (x)
#define ARROW_PREDICT_TRUE(x) (x)
#define ARROW_PREFETCH(addr)
#else
#define ARROW_NORETURN
#define ARROW_PREDICT_FALSE(x) (x)
#define ARROW_PREDICT_TRUE(x) (x)
#define ARROW_PREFETCH(addr)
#endif

#if defined(__GNUC__) || defined(__clang__) || defined(_MSC_VER)
#define ARROW_RESTRICT __restrict
#else
#define ARROW_RESTRICT
#endif

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#endif
