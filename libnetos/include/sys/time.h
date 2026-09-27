/* newlib's, plus the BSD timeval macros and select() */
#include_next <sys/time.h>
#ifndef _NETOS_SYS_TIME_H
#define _NETOS_SYS_TIME_H
#include <sys/select.h>
#ifndef timerisset
#define timerisset(t)		((t)->tv_sec || (t)->tv_usec)
#define timerclear(t)		((t)->tv_sec = (t)->tv_usec = 0)
#define timercmp(a, b, op)	((a)->tv_sec == (b)->tv_sec ? \
				 (a)->tv_usec op (b)->tv_usec : (a)->tv_sec op (b)->tv_sec)
#define timeradd(a, b, r)	do { (r)->tv_sec = (a)->tv_sec + (b)->tv_sec; \
				(r)->tv_usec = (a)->tv_usec + (b)->tv_usec; \
				if ((r)->tv_usec >= 1000000) { (r)->tv_sec++; (r)->tv_usec -= 1000000; } } while (0)
#define timersub(a, b, r)	do { (r)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
				(r)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
				if ((r)->tv_usec < 0) { (r)->tv_sec--; (r)->tv_usec += 1000000; } } while (0)
#endif
#endif
