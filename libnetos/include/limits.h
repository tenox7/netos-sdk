/* newlib's, plus NAME_MAX */
#include_next <limits.h>
#ifndef NAME_MAX
#define NAME_MAX	255
#endif
