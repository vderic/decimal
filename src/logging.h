#ifndef LOGGING_H_
#define LOGGING_H_

#define Insist(assertion)                                                      \
  do {                                                                         \
    if (!(assertion)) {                                                        \
      fprintf(stderr, "Internal Error: %s:%d\n", __FILE__, __LINE__);          \
      abort();                                                                 \
    }                                                                          \
  } while (0)

#define CHECKX(assertion, msg)                                                 \
  do {                                                                         \
    if (!(assertion)) {                                                        \
      fprintf(stderr, "Runtime Error: %s %s:%d\n", msg, __FILE__, __LINE__);   \
      abort();                                                                 \
    }                                                                          \
  } while (0)

#if 0
#define DEC128_IGNORE_EXPR(expr) ((void)(expr))

#define DCHECK(condition) DEC128_IGNORE_EXPR(condition)
#define DCHECK_OK(status) DEC128_IGNORE_EXPR(status)
#define DCHECK_EQ(val1, val2) DEC128_IGNORE_EXPR(val1)
#define DCHECK_NE(val1, val2) DEC128_IGNORE_EXPR(val1)
#define DCHECK_LE(val1, val2) DEC128_IGNORE_EXPR(val1)
#define DCHECK_LT(val1, val2) DEC128_IGNORE_EXPR(val1)
#define DCHECK_GE(val1, val2) DEC128_IGNORE_EXPR(val1)
#define DCHECK_GT(val1, val2) DEC128_IGNORE_EXPR(val1)

#else

#define DCHECK(condition) Insist(condition)
#define DCHECK_OK(status) Insist((status) == 0)
#define DCHECK_EQ(val1, val2) Insist((val1) == (val2))
#define DCHECK_NE(val1, val2) Insist((val1) != (val2))
#define DCHECK_LE(val1, val2) Insist((val1) <= (val2))
#define DCHECK_LT(val1, val2) Insist((val1) < (val2))
#define DCHECK_GE(val1, val2) Insist((val1) >= (val2))
#define DCHECK_GT(val1, val2) Insist((val1) > (val2))

#endif

#endif
