#ifndef _SYS_PARAM_H
#define _SYS_PARAM_H
/* newlib leaves this to the system; these are netOS's 4.3BSD values */
#include <sys/types.h>
#include <endian.h>

#define BSD		198911
#define MAXPATHLEN	1024
#define MAXHOSTNAMELEN	256
#define MAXSYMLINKS	8
#define NOFILE		64
#define NGROUPS		16
#define HZ		1000
#define DEV_BSIZE	512
#define NBBY		8
#ifndef howmany
#define howmany(x, y)	(((x) + ((y) - 1)) / (y))
#endif
#define roundup(x, y)	((((x) + ((y) - 1)) / (y)) * (y))
#define powerof2(x)	((((x) - 1) & (x)) == 0)
#ifndef MIN
#define MIN(a, b)	((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b)	((a) > (b) ? (a) : (b))
#endif
#endif
