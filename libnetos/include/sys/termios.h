#ifndef _SYS_TERMIOS_H
#define _SYS_TERMIOS_H
/* 4.4BSD termios, emulated by libnetos on the netOS pty flags */
typedef unsigned long tcflag_t;
typedef unsigned char cc_t;
typedef long speed_t;

#define VEOF		0
#define VEOL		1
#define VEOL2		2
#define VERASE		3
#define VWERASE		4
#define VKILL		5
#define VREPRINT	6
#define VINTR		8
#define VQUIT		9
#define VSUSP		10
#define VDSUSP		11
#define VSTART		12
#define VSTOP		13
#define VLNEXT		14
#define VDISCARD	15
#define VMIN		16
#define VTIME		17
#define VSTATUS		18
#define NCCS		20

#define IGNBRK		0x00000001
#define BRKINT		0x00000002
#define IGNPAR		0x00000004
#define PARMRK		0x00000008
#define INPCK		0x00000010
#define ISTRIP		0x00000020
#define INLCR		0x00000040
#define IGNCR		0x00000080
#define ICRNL		0x00000100
#define IXON		0x00000200
#define IXOFF		0x00000400
#define IXANY		0x00000800
#define IMAXBEL		0x00002000

#define OPOST		0x00000001
#define ONLCR		0x00000002
#define OXTABS		0x00000004
#define ONOEOT		0x00000008

#define CSIZE		0x00000300
#define CS5		0x00000000
#define CS6		0x00000100
#define CS7		0x00000200
#define CS8		0x00000300
#define CSTOPB		0x00000400
#define CREAD		0x00000800
#define PARENB		0x00001000
#define PARODD		0x00002000
#define HUPCL		0x00004000
#define CLOCAL		0x00008000

#define ECHOKE		0x00000001
#define ECHOE		0x00000002
#define ECHOK		0x00000004
#define ECHO		0x00000008
#define ECHONL		0x00000010
#define ECHOPRT		0x00000020
#define ECHOCTL		0x00000040
#define ISIG		0x00000080
#define ICANON		0x00000100
#define ALTWERASE	0x00000200
#define IEXTEN		0x00000400
#define EXTPROC		0x00000800
#define TOSTOP		0x00400000
#define FLUSHO		0x00800000
#define PENDIN		0x20000000
#define NOFLSH		0x80000000

struct termios {
	tcflag_t	c_iflag;
	tcflag_t	c_oflag;
	tcflag_t	c_cflag;
	tcflag_t	c_lflag;
	cc_t		c_cc[NCCS];
	speed_t		c_ispeed;
	speed_t		c_ospeed;
};

#define TCSANOW		0
#define TCSADRAIN	1
#define TCSAFLUSH	2

#define B0	0
#define B50	50
#define B75	75
#define B110	110
#define B134	134
#define B150	150
#define B200	200
#define B300	300
#define B600	600
#define B1200	1200
#define B1800	1800
#define B2400	2400
#define B4800	4800
#define B9600	9600
#define B19200	19200
#define B38400	38400
#define B57600	57600
#define B115200	115200

#define TCIFLUSH	1
#define TCOFLUSH	2
#define TCIOFLUSH	3
#define TCOOFF		1
#define TCOON		2
#define TCIOFF		3
#define TCION		4

speed_t	cfgetispeed(const struct termios *);
speed_t	cfgetospeed(const struct termios *);
int	cfsetispeed(struct termios *, speed_t);
int	cfsetospeed(struct termios *, speed_t);
void	cfmakeraw(struct termios *);
int	tcgetattr(int, struct termios *);
int	tcsetattr(int, int, const struct termios *);
int	tcdrain(int);
int	tcflow(int, int);
int	tcflush(int, int);
int	tcsendbreak(int, int);
#endif
