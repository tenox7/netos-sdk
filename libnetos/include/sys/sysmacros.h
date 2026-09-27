#ifndef _SYS_SYSMACROS_H
#define _SYS_SYSMACROS_H
/* netOS device numbers are 4.3BSD's: 8-bit major, 8-bit minor */
#define major(d)	(((unsigned) (d) >> 8) & 0xff)
#define minor(d)	((unsigned) (d) & 0xff)
#define makedev(a, i)	((dev_t) (((a) << 8) | (i)))
#endif
