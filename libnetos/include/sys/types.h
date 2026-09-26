#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H
#include <stddef.h>
typedef long off_t;
typedef int ssize_t;
typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;
typedef char *caddr_t;

#define FD_SETSIZE	256
#define NFDBITS		32
typedef struct fd_set { unsigned int fds_bits[FD_SETSIZE / NFDBITS]; } fd_set;
#define FD_SET(n, p)	((p)->fds_bits[(n) / NFDBITS] |= 1U << ((n) % NFDBITS))
#define FD_CLR(n, p)	((p)->fds_bits[(n) / NFDBITS] &= ~(1U << ((n) % NFDBITS)))
#define FD_ISSET(n, p)	((p)->fds_bits[(n) / NFDBITS] & (1U << ((n) % NFDBITS)))
#define FD_ZERO(p)	memset((p), 0, sizeof *(p))
void *memset(void *, int, size_t);
#endif
