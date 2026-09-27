#ifndef _SYS_SELECT_H
#define _SYS_SELECT_H
#include <sys/types.h>
#include <sys/time.h>
int select(int, fd_set *, fd_set *, fd_set *, struct timeval *);
#endif
