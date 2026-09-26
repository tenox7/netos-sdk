#ifndef _SYS_SOCKET_H
#define _SYS_SOCKET_H
#include <sys/types.h>

#define SOCK_STREAM	1
#define SOCK_DGRAM	2
#define SOCK_RAW	3

#define AF_UNSPEC	0
#define AF_UNIX		1
#define AF_INET		2
#define PF_UNSPEC	AF_UNSPEC
#define PF_UNIX		AF_UNIX
#define PF_INET		AF_INET

#define SOL_SOCKET	0xffff
#define SO_DEBUG	0x0001
#define SO_REUSEADDR	0x0004
#define SO_KEEPALIVE	0x0008
#define SO_BROADCAST	0x0020
#define SO_LINGER	0x0080
#define SO_SNDBUF	0x1001
#define SO_RCVBUF	0x1002

#define MSG_OOB		0x1
#define MSG_PEEK	0x2

/* 4.3BSD layout: a 16-bit family and no sa_len */
struct sockaddr {
	u_short	sa_family;
	char	sa_data[14];
};

typedef int socklen_t;

struct linger {
	int	l_onoff;
	int	l_linger;
};

int socket(int, int, int);
int socketpair(int, int, int, int *);
int bind(int, const struct sockaddr *, socklen_t);
int listen(int, int);
int accept(int, struct sockaddr *, socklen_t *);
int connect(int, const struct sockaddr *, socklen_t);
int getsockname(int, struct sockaddr *, socklen_t *);
int getpeername(int, struct sockaddr *, socklen_t *);
int setsockopt(int, int, int, const void *, socklen_t);
int shutdown(int, int);
int recv(int, void *, int, int);
int send(int, const void *, int, int);
int recvfrom(int, void *, int, int, struct sockaddr *, socklen_t *);
int sendto(int, const void *, int, int, const struct sockaddr *, socklen_t);
#endif
