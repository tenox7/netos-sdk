/* syslog through the netOS kernel's own, which feeds its syslogd */
#include <stdarg.h>
#include <stdio.h>
#include <syslog.h>
#include <unistd.h>

extern void netos_syslog(int, const char *, ...);

static const char *ident = "";
static int opt, fac = LOG_USER, mask = 0xff;

void openlog(const char *id, int o, int f)
{
	ident = id ? id : "";
	opt = o;
	if (f) fac = f;
}

void closelog(void) {}

int setlogmask(int m)
{
	int o = mask;

	if (m) mask = m;
	return o;
}

void vsyslog(int pri, const char *fmt, va_list ap)
{
	char b[512];
	int n = 0;

	if (!(mask & LOG_MASK(LOG_PRI(pri)))) return;
	if (*ident) n = snprintf(b, sizeof b, opt & LOG_PID ? "%s[%d]: " : "%s: ", ident, getpid());
	vsnprintf(b + n, sizeof b - n, fmt, ap);
	if (opt & LOG_PERROR) fprintf(stderr, "%s\n", b);
	netos_syslog(pri & LOG_FACMASK ? pri : pri | fac, "%s", b);
}

void syslog(int pri, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vsyslog(pri, fmt, ap);
	va_end(ap);
}
