/* newlib's, plus the POSIX no-match return */
#include_next <glob.h>
#ifndef GLOB_NOMATCH
#define GLOB_NOMATCH	(-3)
#endif
