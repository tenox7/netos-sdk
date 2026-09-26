#ifndef _NETOS_H
#define _NETOS_H
/*
 * Direct access to the C library inside the netOS kernel.
 *
 * netOS carries a full 4.4BSD libc (group 10 has at least 184 entries), so
 * calling it beats linking our own: smaller images, and it has things libnetos
 * lacks.  The kernel is stripped, so each entry had to be identified from how
 * the shipped binaries call it - see fingerprint.sh and the table in README.md.
 *
 * Adding one is two lines: a SYS() line in crt0.s with the group/index, and a
 * declaration here.
 */

/* group 7 - system calls */
int  netos_open(const char *path, int flags, int mode);		/* 7/0x0d */
int  netos_close(int fd);					/* 7/0x02 */
int  netos_read(int fd, void *buf, int n);			/* 7/0x0e */
int  netos_write(int fd, const void *buf, int n);		/* 7/0x19 */
int  netos_stat(const char *path, void *sb);			/* 7/0x23 */
int  netos_lstat(const char *path, void *sb);			/* 7/0x24 */
long netos_lseek(int fd, long off, int whence);			/* 7/0x29 */
int  netos_select(int n, void *r, void *w, void *e, void *tv);	/* 7/0x2c */
int  netos_readlink(const char *path, char *buf, int n);	/* 7/0x45 */
int  netos_rmdir(const char *path);				/* 7/0x47 */
void netos_exit(int status);					/* 7/0x06 */

/* group 10 - C library */
void  netos_bcopy(const void *src, void *dst, int n);		/* 10/0x07 */
void *netos_fopen(const char *path, const char *mode);		/* 10/0x17 */
int   netos_fprintf(void *stream, const char *fmt, ...);	/* 10/0x18 */
void  netos_free(void *p);					/* 10/0x1c */
char *netos_getenv(const char *name);				/* 10/0x20 */
void *netos_malloc(unsigned long n);				/* 10/0x2a */
int   netos_printf(const char *fmt, ...);			/* 10/0x36 */
int   netos_sprintf(char *buf, const char *fmt, ...);		/* 10/0x49 */
int   netos_sscanf(const char *s, const char *fmt, ...);	/* 10/0x4c */
char *netos_strcat(char *dst, const char *src);			/* 10/0x4e */
char *netos_strchr(const char *s, int c);			/* 10/0x4f */
int   netos_strcmp(const char *a, const char *b);		/* 10/0x50 */
char *netos_strcpy(char *dst, const char *src);			/* 10/0x51 */
int   netos_strlen(const char *s);				/* 10/0x54 */
int   netos_strncmp(const char *a, const char *b, int n);	/* 10/0x57 */
long  netos_time(long *tp);					/* 10/0x85 */
void  netos_qsort(void *base, unsigned long n, unsigned long sz, int (*cmp)());	/* 10/0x3a */
int   netos_fputc(int c, void *stream);				/* 10/0x76 */
char *netos_asctime(const void *tm);	/* 10/0x8b - malloc'd, 26 bytes */
char *netos_ctime(const long *tp);	/* 10/0x8c - malloc'd, 26 bytes */
void *netos_gmtime(const long *tp);	/* 10/0x8d - malloc'd struct tm */
void *netos_localtime(const long *tp);	/* 10/0x8e - returns malloc'd tm */
char *netos_strrchr(const char *s, int c);			/* 10/0x8f */
void *netos_gethostbyname(const char *name);	/* 10/0x7c - malloc'd hostent */
void *netos_gethostbyaddr(const void *a, int len, int type);	/* 10/0x7d - malloc'd */

#endif
