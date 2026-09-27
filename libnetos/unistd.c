#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <stddef.h>

extern int netos_open(const char *, int, int);
extern int netos_close(int);
extern int netos_read(int, void *, int);
extern int netos_write(int, const void *, int);
extern long netos_lseek(int, long, int);
extern int netos_select(int, void *, void *, void *, struct timeval *);
extern long netos_time(long *);
extern struct tm *netos_localtime(const long *);
extern void netos_free(void *);

int open(const char *p, int f, ...)
{
	/* mode only matters with O_CREAT; 0666 matches what netOS tools pass */
	return netos_open(p, f, 0666);
}

int close(int fd)			{ return netos_close(fd); }
int read(int fd, void *b, size_t n)	{ return netos_read(fd, b, (int) n); }
int write(int fd, const void *b, size_t n) { return netos_write(fd, b, (int) n); }
long lseek(int fd, long o, int w)	{ return netos_lseek(fd, o, w); }

int select(int n, void *r, void *w, void *e, struct timeval *t)
{
	return netos_select(n, r, w, e, t);
}

int usleep(unsigned long us)
{
	struct timeval tv;

	tv.tv_sec = us / 1000000L;
	tv.tv_usec = us % 1000000L;
	return netos_select(0, 0, 0, 0, &tv);
}

unsigned sleep(unsigned s)
{
	usleep((unsigned long) s * 1000000L);
	return 0;
}

time_t time(time_t *tp)
{
	return netos_time(tp);
}

/* netOS hands back a malloc'd struct tm; copy it and give it straight back */
struct tm *localtime(const time_t *tp)
{
	static struct tm lt;
	struct tm *p = netos_localtime(tp);

	if (!p) return 0;
	memcpy(&lt, p, sizeof lt);
	netos_free(p);
	return &lt;
}

/* The netOS time functions all return malloc'd storage; standard C says these
   return static buffers, so copy and hand the original back. */
extern struct tm *netos_gmtime(const long *);
extern char *netos_ctime(const long *);
extern char *netos_asctime(const struct tm *);

static void *keep(void *p, void *dst, int n)
{
	if (!p) return 0;
	memcpy(dst, p, n);
	netos_free(p);
	return dst;
}

struct tm *gmtime(const time_t *tp)
{
	static struct tm gt;
	return keep(netos_gmtime(tp), &gt, sizeof gt);
}

char *ctime(const time_t *tp)
{
	static char cb[26];
	return keep(netos_ctime(tp), cb, sizeof cb);
}

char *asctime(const struct tm *tm)
{
	static char ab[26];
	return keep(netos_asctime(tm), ab, sizeof ab);
}

/* raw exit: no stdio flush, safe in a vforked child before execv */
extern void netos_exit(int);
void _exit(int code) { netos_exit(code); for (;;) ; }
