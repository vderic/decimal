#ifndef ENDIAN_H_
#define ENDIAN_H_

#ifdef _WIN32
#define DEC128_LITTLE_ENDIAN 1
#else
#if defined(__APPLE__) || defined(__FreeBSD__)
#include <machine/endian.h> // IWYU pragma: keep
#elif defined(sun) || defined(__sun)
#include <sys/byteorder.h> // IWYU pragma: keep
#else
#include <endian.h> // IWYU pragma: keep
#endif
#
#ifndef __BYTE_ORDER__
#error "__BYTE_ORDER__ not defined"
#endif
#
#ifndef __ORDER_LITTLE_ENDIAN__
#error "__ORDER_LITTLE_ENDIAN__ not defined"
#endif
#
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define DEC128_LITTLE_ENDIAN 1
#else
#define DEC128_LITTLE_ENDIAN 0
#endif
#endif

#endif
