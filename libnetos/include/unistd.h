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

int dup(int);
int dup2(int, int);
int pipe(int *);
int execv(const char *, char *const *);
int execl(const char *, const char *, ...);
int vfork(void);		/* netOS has no fork(); a vforked child may
				   only set up fds and execv/_exit */
int getpid(void);
void _exit(int);
int chdir(const char *);

#define STDIN_FILENO	0
#define STDOUT_FILENO	1
#define STDERR_FILENO	2

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif
