/* Terminals.  A netOS tty is one end of a socketpair whose other end is a
   pty program (the kernel's, or telnetd) that does the line discipline.  The
   program toggles two flags the pty reads back: raw (no line editing and no
   LF -> CRLF) and no-echo.  termios is emulated on top of them; output and
   input mapping the kernel skips in raw mode is done here. */
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>

#define TSET	0x6603		/* ioctl: set flags */
#define TCLR	0x6604		/* ioctl: clear flags */
#define TGET	0x6608		/* ioctl: read flags; low bits are FREAD|FWRITE */
#define TRAW	0x4000
#define TNOECHO	0x8000

extern int netos_read(int, void *, int);
extern int netos_write(int, const void *, int);
extern int netos_ioctl(int, unsigned long, void *);
extern int netos_close(int);
extern int netos_dup2(int, int);
extern int netos_fstat(int, void *);
extern int netos_getsockname(int, void *, int *);

static struct termios tio;
static int tset, tfds;		/* tio came from tcsetattr; tty fds among 0..31 */

int isatty(int fd)
{
	int st[16], fl = 0, len = sizeof(struct sockaddr), e = errno;
	struct sockaddr sa;

	if (netos_fstat(fd, st) == 0) goto no;
	netos_ioctl(fd, TGET, &fl);
	if ((fl & 3) != 3) goto no;
	if (netos_getsockname(fd, &sa, &len) == 0 && sa.sa_family == AF_INET) goto no;
	errno = e;
	return 1;
no:
	errno = ENOTTY;
	return 0;
}

static int raw(int fd)
{
	return tset && (unsigned) fd < 32 && (tfds >> fd & 1) && !(tio.c_lflag & ICANON);
}

static void deftio(struct termios *t)
{
	memset(t, 0, sizeof *t);
	t->c_iflag = BRKINT | ICRNL | IXON | IMAXBEL;
	t->c_oflag = OPOST | ONLCR;
	t->c_cflag = CREAD | CS8 | HUPCL;
	t->c_lflag = ECHOKE | ECHOE | ECHOK | ECHO | ECHOCTL | ISIG | ICANON | IEXTEN;
	t->c_cc[VEOF] = 4;
	t->c_cc[VERASE] = 127;
	t->c_cc[VWERASE] = 23;
	t->c_cc[VKILL] = 21;
	t->c_cc[VREPRINT] = 18;
	t->c_cc[VINTR] = 3;
	t->c_cc[VQUIT] = 28;
	t->c_cc[VSUSP] = 26;
	t->c_cc[VSTART] = 17;
	t->c_cc[VSTOP] = 19;
	t->c_cc[VLNEXT] = 22;
	t->c_cc[VDISCARD] = 15;
	t->c_cc[VMIN] = 1;
	t->c_ispeed = t->c_ospeed = B38400;
}

int tcgetattr(int fd, struct termios *t)
{
	int fl = 0;

	if (!isatty(fd)) return -1;
	if (tset) *t = tio;
	else deftio(t);
	netos_ioctl(fd, TGET, &fl);
	t->c_lflag = fl & TRAW ? t->c_lflag & ~ICANON : t->c_lflag | ICANON;
	t->c_lflag = fl & TNOECHO ? t->c_lflag & ~ECHO : t->c_lflag | ECHO;
	return 0;
}

int tcsetattr(int fd, int act, const struct termios *t)
{
	int i, fl = 0;

	if (!isatty(fd)) return -1;
	/* the pty maps LF -> CRLF when it reads, so give it a moment with what
	   was written in cooked mode before raw mode turns that off */
	netos_ioctl(fd, TGET, &fl);
	if (!(fl & TRAW) && !(t->c_lflag & ICANON)) usleep(50000);
	tio = *t;
	tset = 1;
	for (tfds = (unsigned) fd < 32 ? 1 << fd : 0, i = 0; i < 3; i++)
		if (i != fd && isatty(i)) tfds |= 1 << i;
	netos_ioctl(fd, t->c_lflag & ICANON ? TCLR : TSET, (void *) TRAW);
	netos_ioctl(fd, t->c_lflag & ECHO ? TCLR : TSET, (void *) TNOECHO);
	return 0;
}

void cfmakeraw(struct termios *t)
{
	t->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	t->c_oflag &= ~OPOST;
	t->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	t->c_cflag = (t->c_cflag & ~(CSIZE | PARENB)) | CS8;
	t->c_cc[VMIN] = 1;
	t->c_cc[VTIME] = 0;
}

speed_t cfgetispeed(const struct termios *t) { return t->c_ispeed; }
speed_t cfgetospeed(const struct termios *t) { return t->c_ospeed; }
int cfsetispeed(struct termios *t, speed_t s) { t->c_ispeed = s; return 0; }
int cfsetospeed(struct termios *t, speed_t s) { t->c_ospeed = s; return 0; }
int tcdrain(int fd) { return isatty(fd) ? 0 : -1; }
int tcflow(int fd, int a) { return isatty(fd) ? 0 : -1; }
int tcsendbreak(int fd, int d) { return isatty(fd) ? 0 : -1; }
pid_t tcgetpgrp(int fd) { return isatty(fd) ? getpid() : -1; }
int tcsetpgrp(int fd, pid_t p) { return isatty(fd) ? 0 : -1; }

static int ready(int fd, int ds)
{
	struct timeval tv;
	fd_set r;

	FD_ZERO(&r);
	FD_SET(fd, &r);
	tv.tv_sec = ds / 10;
	tv.tv_usec = ds % 10 * 100000;
	return select(fd + 1, &r, 0, 0, &tv) > 0;
}

int tcflush(int fd, int q)
{
	char b[64];

	if (!isatty(fd)) return -1;
	if (q != TCOFLUSH)
		while (ready(fd, 0) && netos_read(fd, b, sizeof b) > 0)
			;
	return 0;
}

int read(int fd, void *buf, size_t n)
{
	unsigned char *b = buf;
	int r, i, k, c;

	if (!raw(fd)) return netos_read(fd, buf, n);
	if (!tio.c_cc[VMIN] && !ready(fd, tio.c_cc[VTIME])) return 0;
	if ((r = netos_read(fd, buf, n)) <= 0) return r;
	for (i = k = 0; i < r; i++) {
		c = b[i];
		if (c == '\r' && (tio.c_iflag & IGNCR)) continue;
		if (c == '\r' && (tio.c_iflag & ICRNL)) c = '\n';
		else if (c == '\n' && (tio.c_iflag & INLCR)) c = '\r';
		if ((tio.c_lflag & ISIG) && c && (c == tio.c_cc[VINTR] || c == tio.c_cc[VQUIT])) {
			raise(c == tio.c_cc[VINTR] ? SIGINT : SIGQUIT);
			continue;
		}
		b[k++] = c;
	}
	if (k) return k;
	errno = EINTR;
	return -1;
}

static int wall(int fd, const char *p, int n)
{
	int r;

	for (; n > 0; p += r, n -= r)
		if ((r = netos_write(fd, p, n)) < 0) return -1;
	return 0;
}

int write(int fd, const void *buf, size_t n)
{
	const char *p = buf, *e = p + n;
	char t[512];
	int k;

	if (!raw(fd) || (tio.c_oflag & (OPOST | ONLCR)) != (OPOST | ONLCR)) return netos_write(fd, buf, n);
	while (p < e) {
		for (k = 0; p < e && k < (int) sizeof t - 1; p++) {
			if (*p == '\n') t[k++] = '\r';
			t[k++] = *p;
		}
		if (wall(fd, t, k)) return -1;
	}
	return n;
}

int close(int fd)
{
	if ((unsigned) fd < 32) tfds &= ~(1 << fd);
	return netos_close(fd);
}

int dup2(int a, int b)
{
	if ((unsigned) b < 32) tfds &= ~(1 << b);
	return netos_dup2(a, b);
}

/* netOS has no window size; LINES and COLUMNS override 24x80 */
static int winsize(int fd, struct winsize *w)
{
	const char *l = getenv("LINES"), *c = getenv("COLUMNS");

	if (!isatty(fd)) return -1;
	w->ws_row = l && atoi(l) > 0 ? atoi(l) : 24;
	w->ws_col = c && atoi(c) > 0 ? atoi(c) : 80;
	w->ws_xpixel = w->ws_ypixel = 0;
	return 0;
}

int ioctl(int fd, unsigned long req, ...)
{
	va_list ap;
	void *a;

	va_start(ap, req);
	a = va_arg(ap, void *);
	va_end(ap);
	switch (req) {
	case TIOCGWINSZ:
		return winsize(fd, a);
	case TIOCGETA:
		return tcgetattr(fd, a);
	case TIOCSETA:
	case TIOCSETAW:
	case TIOCSETAF:
		return tcsetattr(fd, 0, a);
	case TIOCGPGRP:
		if (!isatty(fd)) return -1;
		*(int *) a = getpid();
		return 0;
	case TIOCSWINSZ:
	case TIOCSPGRP:
	case TIOCSCTTY:
	case TIOCNOTTY:
		return isatty(fd) ? 0 : -1;
	}
	return netos_ioctl(fd, req, a);
}

char *ttyname(int fd)
{
	return isatty(fd) ? "/dev/tty" : 0;
}
