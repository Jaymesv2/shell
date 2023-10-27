#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifndef DEBUG
// if DEBUG is not set
#define eprintf(...) {}
#define elog(fmtstr, ...) {}

#else

#include <stdio.h>

#define eprintf(...) fprintf(stderr, __VA_ARGS__) 

#define elog(fmtstr, ...) fprintf(stderr, fmtstr \
    "\tin %s:%d in file %s.\n", __VA_ARGS__ __VA_OPT__(,) __func__, __LINE__, __FILE__)


#endif

#endif