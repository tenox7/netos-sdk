#ifndef _SYS_TIME_H
#define _SYS_TIME_H
struct timeval { long tv_sec, tv_usec; };
int select(int, void *, void *, void *, struct timeval *);
#endif
