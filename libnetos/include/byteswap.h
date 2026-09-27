#ifndef _BYTESWAP_H
#define _BYTESWAP_H
#define bswap_16(x)	((unsigned short) ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8)))
#define bswap_32(x)	((((x) & 0xff000000UL) >> 24) | (((x) & 0x00ff0000UL) >> 8) | \
			 (((x) & 0x0000ff00UL) << 8) | (((x) & 0x000000ffUL) << 24))
#define bswap_64(x)	((((unsigned long long) bswap_32((unsigned long) (x))) << 32) | \
			 bswap_32((unsigned long) ((x) >> 32)))
#endif
