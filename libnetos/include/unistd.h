#ifndef _UNISTD_H
#define _UNISTD_H
#include <stddef.h>
int open(const char *, int, ...);
int close(int);
int read(int, void *, size_t);
int write(int, const void *, size_t);
long lseek(int, long, int);
unsigned sleep(unsigned);
int usleep(unsigned long);
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif
