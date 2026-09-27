#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/utsname.h>

int gethostname(char *b, size_t n)
{
	const char *h = getenv("HOSTNAME");

	strncpy(b, h ? h : "netos", n);
	return 0;
}

int uname(struct utsname *u)
{
	memset(u, 0, sizeof *u);
	strcpy(u->sysname, "netOS");
	gethostname(u->nodename, sizeof u->nodename - 1);
	strcpy(u->release, "3.2");
	strcpy(u->version, "netos-sdk");
	strcpy(u->machine, "i960");
	return 0;
}

long sysconf(int n)
{
	switch (n) {
	case _SC_ARG_MAX: return 4096;
	case _SC_CHILD_MAX: return 16;
	case _SC_CLK_TCK: return CLOCKS_PER_SEC;
	case _SC_OPEN_MAX: return 64;
	case _SC_PAGESIZE: return 4096;
	}
	errno = EINVAL;
	return -1;
}

long fpathconf(int fd, int n) { return n == _PC_NAME_MAX ? MAXNAMLEN : n == _PC_PATH_MAX ? PATH_MAX : 255; }
long pathconf(const char *p, int n) { return fpathconf(-1, n); }
int getpagesize(void) { return 4096; }
int getdtablesize(void) { return 64; }

/* netOS has no data segment to grow; old programs that sbrk get a 1 MB
   zeroed arena from the heap instead */
#define ARENA (1024 * 1024)

void *sbrk(ptrdiff_t n)
{
	static char *base, *brk;
	char *p;

	if (!base && !(brk = base = calloc(1, ARENA))) goto fail;
	if (n > base + ARENA - brk || n < base - brk) goto fail;
	p = brk;
	brk += n;
	return p;
fail:
	errno = ENOMEM;
	return (void *) -1;
}

int sethostname(const char *n, size_t len)
{
	errno = EPERM;
	return -1;
}

int getdomainname(char *b, size_t n)
{
	if (n) *b = 0;
	return 0;
}

/* canonical absolute path: "." and ".." gone, symlinks left alone (netOS
   readlink hangs the Station) */
char *realpath(const char *path, char *out)
{
	char buf[PATH_MAX], *q;
	const char *p, *e;
	struct stat st;
	int n;

	if (*path != '/') {
		if (!getcwd(buf, sizeof buf)) return 0;
	} else
		*buf = 0;
	for (p = path; *p; p = *e ? e + 1 : e) {
		if (!(e = strchr(p, '/'))) e = p + strlen(p);
		n = e - p;
		if (!n || (n == 1 && *p == '.')) continue;
		if (n == 2 && p[0] == '.' && p[1] == '.') {
			if ((q = strrchr(buf, '/'))) *q = 0;
			continue;
		}
		q = buf + strlen(buf);
		if (q - buf + n + 2 > PATH_MAX) {
			errno = ENAMETOOLONG;
			return 0;
		}
		*q = '/';
		memcpy(q + 1, p, n);
		q[n + 1] = 0;
	}
	if (!*buf) strcpy(buf, "/");
	if (stat(buf, &st)) return 0;
	if (!out && !(out = malloc(strlen(buf) + 1))) return 0;
	return strcpy(out, buf);
}
