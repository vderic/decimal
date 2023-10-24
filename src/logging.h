#ifndef LOGGING_H_
#define LOGGING_H_

#define ARROW_IGNORE_EXPR(expr) ((void)(expr))

#define DCHECK(condition) ARROW_IGNORE_EXPR(condition)
#define DCHECK_OK(status) ARROW_IGNORE_EXPR(status)
#define DCHECK_EQ(val1, val2) ARROW_IGNORE_EXPR(val1)
#define DCHECK_NE(val1, val2) ARROW_IGNORE_EXPR(val1)
#define DCHECK_LE(val1, val2) ARROW_IGNORE_EXPR(val1)
#define DCHECK_LT(val1, val2) ARROW_IGNORE_EXPR(val1)
#define DCHECK_GE(val1, val2) ARROW_IGNORE_EXPR(val1)
#define DCHECK_GT(val1, val2) ARROW_IGNORE_EXPR(val1)

#endif
