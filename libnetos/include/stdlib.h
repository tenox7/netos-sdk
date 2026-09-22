#ifndef _STDLIB_H
#define _STDLIB_H
#include <stddef.h>
void *malloc(size_t);
void *calloc(size_t, size_t);
void *realloc(void *, size_t);
void free(void *);
void exit(int);
int atoi(const char *);
long atol(const char *);
long strtol(const char *, char **, int);
int abs(int);
char *getenv(const char *);
int rand(void);
void srand(unsigned);
void qsort(void *, size_t, size_t, int (*)(const void *, const void *));	/* netOS 10/0x3a */
#define RAND_MAX 32767
#endif
