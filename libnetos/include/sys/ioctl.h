#ifndef _SYS_IOCTL_H
#define _SYS_IOCTL_H
/* netOS numbers these ('f' << 8 | n), not with BSD's _IOW encoding */
#define FIONREAD	0x6601
#define FIONBIO		0x6602
int ioctl(int, int, ...);
#endif
