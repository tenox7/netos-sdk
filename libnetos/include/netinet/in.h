#ifndef _NETINET_IN_H
#define _NETINET_IN_H
#include <sys/types.h>

#define IPPROTO_IP	0
#define IPPROTO_ICMP	1
#define IPPROTO_TCP	6
#define IPPROTO_UDP	17

/* original Deering multicast numbering, not 4.4BSD's */
#define IP_MULTICAST_IF		2
#define IP_MULTICAST_TTL	3
#define IP_ADD_MEMBERSHIP	5

typedef unsigned int in_addr_t;

#define INADDR_ANY		((in_addr_t) 0x00000000)
#define INADDR_LOOPBACK		((in_addr_t) 0x7f000001)
#define INADDR_BROADCAST	((in_addr_t) 0xffffffff)
#define INADDR_NONE		((in_addr_t) 0xffffffff)

struct in_addr {
	in_addr_t s_addr;
};

struct sockaddr_in {
	u_short	sin_family;
	u_short	sin_port;
	struct	in_addr sin_addr;
	char	sin_zero[8];
};

struct ip_mreq {
	struct	in_addr imr_multiaddr;
	struct	in_addr imr_interface;
};

/* the i960 is little-endian */
#define ntohs(x)	((u_short) ((((x) & 0xff) << 8) | (((x) >> 8) & 0xff)))
#define htons(x)	ntohs(x)
#define ntohl(x)	((in_addr_t) ((((x) & 0xffU) << 24) | (((x) & 0xff00U) << 8) | \
			 (((x) >> 8) & 0xff00U) | (((x) >> 24) & 0xffU)))
#define htonl(x)	ntohl(x)
#endif
