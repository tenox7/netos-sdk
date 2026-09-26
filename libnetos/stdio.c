#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

static FILE _stdin  = { 0, _F_READ,  0, { 0 } };
static FILE _stdout = { 1, _F_WRITE | _F_LINEBUF, 0, { 0 } };
static FILE _stderr = { 2, _F_WRITE, 0, { 0 } };

FILE *stdin = &_stdin, *stdout = &_stdout, *stderr = &_stderr;

static FILE *open_files[8];

int fflush(FILE *f)
{
	int n;

	if (!f) {
		fflush(stdout);
		fflush(stderr);
		return 0;
	}
	if (!f->cnt) return 0;
	n = write(f->fd, f->buf, f->cnt);
	f->cnt = 0;
	return n < 0 ? EOF : 0;
}

void __netos_stdio_cleanup(void)
{
	int i;

	fflush(stdout);
	fflush(stderr);
	for (i = 0; i < 8; i++)
		if (open_files[i]) fflush(open_files[i]);
}

int fputc(int c, FILE *f)
{
	if (!(f->flags & _F_WRITE)) return EOF;
	f->buf[f->cnt++] = (char) c;
	if (f->cnt == BUFSIZ) goto flush;
	if (f == stderr) goto flush;				/* unbuffered */
	if ((f->flags & _F_LINEBUF) && c == '\n') goto flush;
	return c;
flush:
	return fflush(f) == EOF ? EOF : c;
}

int putchar(int c) { return fputc(c, stdout); }

int fputs(const char *s, FILE *f)
{
	while (*s)
		if (fputc(*s++, f) == EOF) return EOF;
	return 0;
}

int puts(const char *s)
{
	if (fputs(s, stdout) == EOF) return EOF;
	return fputc('\n', stdout) == EOF ? EOF : 0;
}

int fgetc(FILE *f)
{
	char c;

	if (!(f->flags & _F_READ)) return EOF;
	if (read(f->fd, &c, 1) != 1) { f->flags |= _F_EOF; return EOF; }
	return (unsigned char) c;
}

int getchar(void) { return fgetc(stdin); }
int feof(FILE *f) { return (f->flags & _F_EOF) != 0; }

char *fgets(char *s, int n, FILE *f)
{
	int c, i = 0;

	while (i < n - 1) {
		if ((c = fgetc(f)) == EOF) break;
		s[i++] = (char) c;
		if (c == '\n') break;
	}
	if (!i) return 0;
	s[i] = 0;
	return s;
}

size_t fread(void *p, size_t sz, size_t n, FILE *f)
{
	int r = read(f->fd, p, sz * n);
	return r <= 0 ? 0 : r / sz;
}

size_t fwrite(const void *p, size_t sz, size_t n, FILE *f)
{
	const char *q = p;
	size_t i, total = sz * n;

	for (i = 0; i < total; i++)
		if (fputc(q[i], f) == EOF) break;
	return sz ? i / sz : 0;
}

FILE *fopen(const char *path, const char *mode)
{
	FILE *f;
	int fd, flags, i;

	if (*mode == 'r')      flags = O_RDONLY;
	else if (*mode == 'w') flags = O_WRONLY | O_CREAT | O_TRUNC;
	else if (*mode == 'a') flags = O_WRONLY | O_CREAT | O_APPEND;
	else return 0;

	if ((fd = open(path, flags)) < 0) return 0;
	if (!(f = malloc(sizeof *f))) { close(fd); return 0; }
	f->fd = fd;
	f->flags = (*mode == 'r') ? _F_READ : _F_WRITE;
	f->cnt = 0;
	for (i = 0; i < 8; i++)
		if (!open_files[i]) { open_files[i] = f; break; }
	return f;
}

int fclose(FILE *f)
{
	int i;

	if (!f) return EOF;
	fflush(f);
	close(f->fd);
	for (i = 0; i < 8; i++)
		if (open_files[i] == f) open_files[i] = 0;
	free(f);
	return 0;
}

void perror(const char *s)
{
	if (s && *s) { fputs(s, stderr); fputs(": ", stderr); }
	fputs(strerror(errno), stderr);
	fputc('\n', stderr);
}
