/* Host implementations of the netOS syscalls libnetos sits on. */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/select.h>
#include <string.h>

struct ntm {
	int tm_sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year;
	int tm_wday, tm_yday, tm_isdst;
	long tm_gmtoff;
	char *tm_zone;
};
struct ntv { long tv_sec, tv_usec; };

int  netos_open(const char *p, int f, int m) { return open(p, f, m); }
int  netos_close(int fd)                     { return close(fd); }
int  netos_read(int fd, void *b, int n)      { return read(fd, b, n); }
int  netos_write(int fd, const void *b, int n) { return write(fd, b, n); }
long netos_lseek(int fd, long o, int w)      { return lseek(fd, o, w); }
void netos_exit(int c)                       { _exit(c); }
void netos_free(void *p)                     { free(p); }

int netos_select(int n, void *r, void *w, void *e, struct ntv *t)
{
	struct timeval tv;
	if (!t) return 0;
	tv.tv_sec = t->tv_sec; tv.tv_usec = t->tv_usec;
	return select(n, 0, 0, 0, &tv);
}

long netos_time(long *p) { time_t t = time(0); if (p) *p = t; return (long) t; }

/* netOS hands back a malloc'd struct tm, so the host stub must too */
void *netos_localtime(const long *tp)
{
	time_t tt = *tp;
	struct tm *h = localtime(&tt);
	struct ntm *n = malloc(sizeof *n);
	n->tm_sec = h->tm_sec; n->tm_min = h->tm_min; n->tm_hour = h->tm_hour;
	n->tm_mday = h->tm_mday; n->tm_mon = h->tm_mon; n->tm_year = h->tm_year;
	n->tm_wday = h->tm_wday; n->tm_yday = h->tm_yday; n->tm_isdst = h->tm_isdst;
	n->tm_gmtoff = 0; n->tm_zone = 0;
	return n;
}

extern int  np_main(int, char **);
extern void __netos_setargs(char **, char **);

int main(int argc, char **argv, char **envp)
{
	__netos_setargs(argv, envp);
	return np_main(argc, argv);
}

void *netos_malloc(unsigned long n) { return malloc(n); }

/* the netOS time family all return malloc'd storage */
void *netos_gmtime(const long *tp)
{
	time_t tt = *tp;
	struct tm *h = gmtime(&tt);
	struct ntm *n = malloc(sizeof *n);
	n->tm_sec = h->tm_sec; n->tm_min = h->tm_min; n->tm_hour = h->tm_hour;
	n->tm_mday = h->tm_mday; n->tm_mon = h->tm_mon; n->tm_year = h->tm_year;
	n->tm_wday = h->tm_wday; n->tm_yday = h->tm_yday; n->tm_isdst = h->tm_isdst;
	n->tm_gmtoff = 0; n->tm_zone = 0;
	return n;
}
char *netos_ctime(const long *tp)
{
	time_t tt = *tp;
	char *p = malloc(26);
	memcpy(p, ctime(&tt), 26);
	return p;
}
char *netos_asctime(const void *tm)
{
	char *p = malloc(26);
	memcpy(p, "Thu Jan  1 00:00:00 1970\n", 26);
	(void) tm;
	return p;
}
