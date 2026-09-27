#ifndef _SIGNAL_H
#define _SIGNAL_H
/* 4.x BSD signal numbers */
#define SIGHUP	1
#define SIGINT	2
#define SIGQUIT	3
#define SIGKILL	9
#define SIGPIPE	13
#define SIGALRM	14
#define SIGTERM	15
#define SIGCHLD	20
#define SIG_DFL	((void (*)(int)) 0)
#define SIG_IGN	((void (*)(int)) 1)
int kill(int, int);
#endif
