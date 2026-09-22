#include <stdio.h>
#include <string.h>

/* Floating point goes through libgcc soft float (fp-bit.c); the i960 core has
   no FPU.  %f and %e are supported, %g maps to %f.  The integer part must fit
   in a long, which is the usual embedded-printf limitation. */

struct out {
	FILE *f;		/* stream sink, or */
	char *buf;		/* buffer sink */
	size_t cap, len;	/* buffer capacity / bytes produced */
};

static void emit(struct out *o, int c)
{
	o->len++;
	if (o->f) { fputc(c, o->f); return; }
	if (o->buf && o->len <= o->cap) o->buf[o->len - 1] = (char) c;
}

static void pad(struct out *o, int n, int c)
{
	while (n-- > 0) emit(o, c);
}

static int unum(char *t, unsigned long v, int base, int upper)
{
	const char *dig = upper ? "0123456789ABCDEF" : "0123456789abcdef";
	int i = 0;

	if (!v) t[i++] = '0';
	while (v) { t[i++] = dig[v % base]; v /= base; }
	return i;
}

/* %f / %e.  Only add, multiply, compare and the double->long conversion are
   needed, all of which libgcc's fp-bit provides. */
static void fnum(struct out *o, double v, int prec, int width, int left, int plus)
{
	char t[40];
	int i, n = 0, neg = 0, d;
	double fp, r;
	long ip;

	if (v < 0) { neg = 1; v = -v; }
	if (prec < 0) prec = 6;
	if (v >= 2147483647.0) {			/* past what a long holds */
		if (neg) emit(o, '-');
		emit(o, 'h'); emit(o, 'u'); emit(o, 'g'); emit(o, 'e');
		return;
	}

	for (r = 0.5, i = 0; i < prec; i++) r *= 0.1;	/* round at the last digit */
	v += r;
	ip = (long) v;
	fp = v - (double) ip;

	if (!ip) t[n++] = '0';
	while (ip > 0) { t[n++] = (char) ('0' + (int) (ip % 10)); ip /= 10; }

	i = n + (neg || plus) + (prec ? prec + 1 : 0);
	if (!left) pad(o, width - i, ' ');
	if (neg) emit(o, '-'); else if (plus) emit(o, '+');
	while (n--) emit(o, t[n]);
	if (prec) {
		emit(o, '.');
		while (prec--) {
			fp *= 10.0;
			d = (int) fp;
			emit(o, (char) ('0' + d));
			fp -= (double) d;
		}
	}
	if (left) pad(o, width - i, ' ');
}

static int core(struct out *o, const char *fmt, va_list ap)
{
	char t[36];
	int n, i, width, prec, left, zero, plus, lng;
	double dv;
	const char *s;
	long sv;
	unsigned long uv;

	for (; *fmt; fmt++) {
		if (*fmt != '%') { emit(o, *fmt); continue; }
		fmt++;
		left = zero = plus = lng = 0;
		for (;; fmt++) {
			if (*fmt == '-') left = 1;
			else if (*fmt == '0') zero = 1;
			else if (*fmt == '+') plus = 1;
			else if (*fmt == ' ') ;
			else break;
		}
		width = 0;
		if (*fmt == '*') { width = va_arg(ap, int); fmt++; }
		else while (*fmt >= '0' && *fmt <= '9') width = width * 10 + (*fmt++ - '0');
		prec = -1;
		if (*fmt == '.') {
			fmt++;
			prec = 0;
			if (*fmt == '*') { prec = va_arg(ap, int); fmt++; }
			else while (*fmt >= '0' && *fmt <= '9') prec = prec * 10 + (*fmt++ - '0');
		}
		while (*fmt == 'l' || *fmt == 'h') { if (*fmt == 'l') lng = 1; fmt++; }

		switch (*fmt) {
		case 'c':
			n = 1;
			if (!left) pad(o, width - n, ' ');
			emit(o, va_arg(ap, int));
			if (left) pad(o, width - n, ' ');
			break;
		case 's':
			s = va_arg(ap, const char *);
			if (!s) s = "(null)";
			n = strlen(s);
			if (prec >= 0 && prec < n) n = prec;
			if (!left) pad(o, width - n, ' ');
			for (i = 0; i < n; i++) emit(o, s[i]);
			if (left) pad(o, width - n, ' ');
			break;
		case 'd':
		case 'i':
			sv = lng ? va_arg(ap, long) : (long) va_arg(ap, int);
			uv = sv < 0 ? (unsigned long) -sv : (unsigned long) sv;
			n = unum(t, uv, 10, 0);
			i = n + (sv < 0 || plus);
			if (!left && zero) {
				if (sv < 0) emit(o, '-'); else if (plus) emit(o, '+');
				pad(o, width - i, '0');
			} else {
				if (!left) pad(o, width - i, ' ');
				if (sv < 0) emit(o, '-'); else if (plus) emit(o, '+');
			}
			while (n--) emit(o, t[n]);
			if (left) pad(o, width - i, ' ');
			break;
		case 'u':
		case 'x':
		case 'X':
		case 'o':
			uv = lng ? va_arg(ap, unsigned long) : (unsigned long) va_arg(ap, unsigned);
			n = unum(t, uv, *fmt == 'o' ? 8 : (*fmt == 'u' ? 10 : 16), *fmt == 'X');
			if (!left) pad(o, width - n, zero ? '0' : ' ');
			for (i = n; i > 0; i--) emit(o, t[i - 1]);
			if (left) pad(o, width - n, ' ');
			break;
		case 'f': case 'F': case 'g': case 'G':
			fnum(o, va_arg(ap, double), prec, width, left, plus);
			break;
		case 'e': case 'E': {
			double dv = va_arg(ap, double);
			int e = 0;
			if (dv != 0.0) {
				while (dv >= 10.0 || dv <= -10.0) { dv *= 0.1; e++; }
				while (dv < 1.0 && dv > -1.0)     { dv *= 10.0; e--; }
			}
			fnum(o, dv, prec, 0, 0, plus);
			emit(o, *fmt == 'E' ? 'E' : 'e');
			emit(o, e < 0 ? '-' : '+');
			if (e < 0) e = -e;
			emit(o, (char) ('0' + e / 10));
			emit(o, (char) ('0' + e % 10));
			break;
		}
		case 'p':
			uv = (unsigned long) va_arg(ap, void *);
			emit(o, '0'); emit(o, 'x');
			n = unum(t, uv, 16, 0);
			while (n--) emit(o, t[n]);
			break;
		case '%':
			emit(o, '%');
			break;
		default:
			emit(o, '%');
			if (*fmt) emit(o, *fmt);
			break;
		}
	}
	return (int) o->len;
}

int vfprintf(FILE *f, const char *fmt, va_list ap)
{
	struct out o;
	o.f = f; o.buf = 0; o.cap = o.len = 0;
	return core(&o, fmt, ap);
}

int vsnprintf(char *b, size_t cap, const char *fmt, va_list ap)
{
	struct out o;
	int n;

	o.f = 0; o.buf = b; o.cap = cap ? cap - 1 : 0; o.len = 0;
	n = core(&o, fmt, ap);
	if (b && cap) b[o.len < o.cap ? o.len : o.cap] = 0;
	return n;
}

int fprintf(FILE *f, const char *fmt, ...)
{
	va_list ap; int n;
	va_start(ap, fmt); n = vfprintf(f, fmt, ap); va_end(ap);
	return n;
}

int printf(const char *fmt, ...)
{
	va_list ap; int n;
	va_start(ap, fmt); n = vfprintf(stdout, fmt, ap); va_end(ap);
	return n;
}

int sprintf(char *b, const char *fmt, ...)
{
	va_list ap; int n;
	va_start(ap, fmt); n = vsnprintf(b, (size_t) -1, fmt, ap); va_end(ap);
	return n;
}

int snprintf(char *b, size_t cap, const char *fmt, ...)
{
	va_list ap; int n;
	va_start(ap, fmt); n = vsnprintf(b, cap, fmt, ap); va_end(ap);
	return n;
}
