/* BSD err(3) */
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char **__netos_argv;

static void msg(int e, const char *fmt, va_list ap)
{
	const char *p = __netos_argv && *__netos_argv ? *__netos_argv : "";

	fprintf(stderr, "%s: ", strrchr(p, '/') ? strrchr(p, '/') + 1 : p);
	if (fmt) vfprintf(stderr, fmt, ap);
	if (e >= 0) fprintf(stderr, fmt ? ": %s" : "%s", strerror(e));
	fputc('\n', stderr);
}

void vwarn(const char *fmt, va_list ap) { msg(errno, fmt, ap); }
void vwarnx(const char *fmt, va_list ap) { msg(-1, fmt, ap); }
void verr(int x, const char *fmt, va_list ap) { msg(errno, fmt, ap); exit(x); }
void verrx(int x, const char *fmt, va_list ap) { msg(-1, fmt, ap); exit(x); }

void warn(const char *fmt, ...) { va_list ap; va_start(ap, fmt); vwarn(fmt, ap); va_end(ap); }
void warnx(const char *fmt, ...) { va_list ap; va_start(ap, fmt); vwarnx(fmt, ap); va_end(ap); }
void err(int x, const char *fmt, ...) { va_list ap; va_start(ap, fmt); verr(x, fmt, ap); va_end(ap); }
void errx(int x, const char *fmt, ...) { va_list ap; va_start(ap, fmt); verrx(x, fmt, ap); va_end(ap); }
