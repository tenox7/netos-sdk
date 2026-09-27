/* newlib's, plus what netOS programs expect alongside it */
#include_next <unistd.h>
#ifndef _NETOS_UNISTD_H
#define _NETOS_UNISTD_H
int	sethostname(const char *, size_t);
int	getdomainname(char *, size_t);
#endif
