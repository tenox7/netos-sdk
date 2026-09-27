#ifndef _SYS_IOCTL_H
#define _SYS_IOCTL_H
/* netOS numbers its own ioctls ('f' << 8 | n), not with BSD's _IOW encoding */
#define FIONREAD	0x6601
#define FIONBIO		0x6602
#define FIOASYNC	0x6605
#define FIOSETOWN	0x6606

/* terminal ioctls, emulated by libnetos */
#define TIOCGETA	0x402c7413
#define TIOCSETA	0x802c7414
#define TIOCSETAW	0x802c7415
#define TIOCSETAF	0x802c7416
#define TIOCGWINSZ	0x40087468
#define TIOCSWINSZ	0x80087467
#define TIOCGPGRP	0x40047477
#define TIOCSPGRP	0x80047476
#define TIOCSCTTY	0x20007461
#define TIOCNOTTY	0x20007471

struct winsize {
	unsigned short	ws_row;
	unsigned short	ws_col;
	unsigned short	ws_xpixel;
	unsigned short	ws_ypixel;
};

int ioctl(int, unsigned long, ...);
#endif
