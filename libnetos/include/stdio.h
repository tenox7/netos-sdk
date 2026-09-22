#ifndef _STDIO_H
#define _STDIO_H
#include <stddef.h>
#include <stdarg.h>

#define EOF	(-1)
#define BUFSIZ	1024

typedef struct _FILE {
	int  fd;
	int  flags;
	int  cnt;			/* bytes currently in buf */
	char buf[BUFSIZ];
} FILE;

#define _F_READ		0x01
#define _F_WRITE	0x02
#define _F_LINEBUF	0x04
#define _F_EOF		0x08
#define _F_ERR		0x10

extern FILE *stdin, *stdout, *stderr;

int printf(const char *, ...);
int fprintf(FILE *, const char *, ...);
int sprintf(char *, const char *, ...);
int snprintf(char *, size_t, const char *, ...);
int vfprintf(FILE *, const char *, va_list);
int vsnprintf(char *, size_t, const char *, va_list);
int sscanf(const char *, const char *, ...);	/* netOS 10/0x4c, used directly */
int puts(const char *);
int fputs(const char *, FILE *);
int fputc(int, FILE *);
int putchar(int);
int fgetc(FILE *);
int getchar(void);
char *fgets(char *, int, FILE *);
size_t fread(void *, size_t, size_t, FILE *);
size_t fwrite(const void *, size_t, size_t, FILE *);
FILE *fopen(const char *, const char *);
int fclose(FILE *);
int fflush(FILE *);
int feof(FILE *);
void perror(const char *);

#define putc(c, f)	fputc((c), (f))
#define getc(f)		fgetc(f)
#endif
