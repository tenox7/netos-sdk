#ifndef _ERR_H
#define _ERR_H
#include <stdarg.h>
void	err(int, const char *, ...);
void	errx(int, const char *, ...);
void	warn(const char *, ...);
void	warnx(const char *, ...);
void	verr(int, const char *, va_list);
void	verrx(int, const char *, va_list);
void	vwarn(const char *, va_list);
void	vwarnx(const char *, va_list);
#endif
