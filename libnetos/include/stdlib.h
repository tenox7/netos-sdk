/* newlib's, plus realpath */
#include_next <stdlib.h>
#ifndef _NETOS_STDLIB_H
#define _NETOS_STDLIB_H
char	*realpath(const char *, char *);
#endif
