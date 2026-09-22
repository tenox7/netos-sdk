#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* netOS has a real heap (10/0x67 malloc, 10/0x1c free), so use it.  We keep an
   8-byte header holding the payload size, which realloc needs and the kernel
   does not hand back. */
extern void *netos_malloc(size_t);
extern void netos_free(void *);

#define HDR 8

void *malloc(size_t n)
{
	char *p;

	if (!n) return 0;
	if (!(p = netos_malloc(n + HDR))) return 0;
	*(size_t *) p = n;
	return p + HDR;
}

void free(void *p)
{
	if (p) netos_free((char *) p - HDR);
}

void *calloc(size_t a, size_t b)
{
	void *p = malloc(a * b);
	if (p) memset(p, 0, a * b);
	return p;
}

void *realloc(void *p, size_t n)
{
	size_t old;
	void *q;

	if (!p) return malloc(n);
	old = *(size_t *) ((char *) p - HDR);
	if (old >= n) return p;
	if (!(q = malloc(n))) return 0;
	memcpy(q, p, old);
	free(p);
	return q;
}

long strtol(const char *s, char **end, int base)
{
	long v = 0;
	int neg = 0, d;

	while (*s == ' ' || (*s >= 9 && *s <= 13)) s++;
	if (*s == '-') { neg = 1; s++; } else if (*s == '+') s++;
	if (!base) {
		if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) { base = 16; s += 2; }
		else if (s[0] == '0') base = 8;
		else base = 10;
	} else if (base == 16 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) s += 2;

	for (;; s++) {
		if (*s >= '0' && *s <= '9') d = *s - '0';
		else if (*s >= 'a' && *s <= 'z') d = *s - 'a' + 10;
		else if (*s >= 'A' && *s <= 'Z') d = *s - 'A' + 10;
		else break;
		if (d >= base) break;
		v = v * base + d;
	}
	if (end) *end = (char *) s;
	return neg ? -v : v;
}

int atoi(const char *s) { return (int) strtol(s, 0, 10); }
long atol(const char *s) { return strtol(s, 0, 10); }
int abs(int n) { return n < 0 ? -n : n; }

static unsigned long seed = 1;
void srand(unsigned s) { seed = s; }
int rand(void)
{
	seed = seed * 1103515245 + 12345;
	return (int) ((seed >> 16) & 0x7fff);
}

/* argv/envp are handed to us by crt0 before main runs */
static char **environ;
void __netos_setargs(char **argv, char **envp) { (void) argv; environ = envp; }

char *getenv(const char *name)
{
	size_t n = strlen(name);
	char **e;

	for (e = environ; e && *e; e++)
		if (!strncmp(*e, name, n) && (*e)[n] == '=')
			return *e + n + 1;
	return 0;
}

extern void __netos_stdio_cleanup(void);
extern void netos_exit(int);

void exit(int code)
{
	__netos_stdio_cleanup();
	netos_exit(code);
	for (;;) ;
}
