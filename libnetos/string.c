#include <string.h>

size_t strlen(const char *s)
{
	const char *p = s;
	while (*p) p++;
	return p - s;
}

char *strcpy(char *d, const char *s)
{
	char *r = d;
	while ((*d++ = *s++)) ;
	return r;
}

char *strncpy(char *d, const char *s, size_t n)
{
	char *r = d;
	while (n && (*d++ = *s++)) n--;
	while (n--) *d++ = 0;
	return r;
}

char *strcat(char *d, const char *s)
{
	strcpy(d + strlen(d), s);
	return d;
}

char *strncat(char *d, const char *s, size_t n)
{
	char *p = d + strlen(d);
	while (n-- && *s) *p++ = *s++;
	*p = 0;
	return d;
}

int strcmp(const char *a, const char *b)
{
	while (*a && *a == *b) { a++; b++; }
	return (unsigned char) *a - (unsigned char) *b;
}

int strncmp(const char *a, const char *b, size_t n)
{
	while (n && *a && *a == *b) { a++; b++; n--; }
	if (!n) return 0;
	return (unsigned char) *a - (unsigned char) *b;
}

char *strchr(const char *s, int c)
{
	do { if (*s == (char) c) return (char *) s; } while (*s++);
	return 0;
}

char *strrchr(const char *s, int c)
{
	const char *r = 0;
	do { if (*s == (char) c) r = s; } while (*s++);
	return (char *) r;
}

char *strstr(const char *h, const char *n)
{
	size_t l = strlen(n);
	if (!l) return (char *) h;
	for (; *h; h++)
		if (!strncmp(h, n, l)) return (char *) h;
	return 0;
}

void *memset(void *d, int c, size_t n)
{
	unsigned char *p = d;
	while (n--) *p++ = (unsigned char) c;
	return d;
}

void *memcpy(void *d, const void *s, size_t n)
{
	unsigned char *p = d;
	const unsigned char *q = s;
	while (n--) *p++ = *q++;
	return d;
}

void *memmove(void *d, const void *s, size_t n)
{
	unsigned char *p = d;
	const unsigned char *q = s;

	if (p == q || !n) return d;
	if (p < q) { while (n--) *p++ = *q++; return d; }
	p += n; q += n;
	while (n--) *--p = *--q;
	return d;
}

int memcmp(const void *a, const void *b, size_t n)
{
	const unsigned char *p = a, *q = b;
	while (n--) { if (*p != *q) return *p - *q; p++; q++; }
	return 0;
}

/* BSD order: source first */
void bcopy(const void *s, void *d, size_t n) { memmove(d, s, n); }
void bzero(void *d, size_t n) { memset(d, 0, n); }
