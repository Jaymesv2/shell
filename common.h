#ifndef _COMMON_H_
#define _COMMON_H_

// #define _POSIX_C_SOURCE 200809
// #define _XOPEN_SOURCE 700
//  I LOVE C


// NOTE: I havent permutation tested these so some combinations might not work properly.
//          I only really test having them all on or just LOOP_OPERATOR
// feature flags

// globbing support
#define GLOBBING 1
// quoted input support
#define QUOTING 1
// escape characters in the input
#define ESCAPE_CHARS 1

// Debug logging
#define DEBUG 1

// configuration
#define MAX_QUOTES 512
#define MAX_QUOTE_LEN 150
#define ARG_MAX 500


// constants
#define PREAD 0
#define PWRITE 1

// helpful macros
#define UNUSED(x) (void)(x)

// these are used for documentation
#define _Nullable
#define _Nonnull

#include "debug.h"


#endif