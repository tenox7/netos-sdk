#ifndef _NETINET_IP_H
#define _NETINET_IP_H
/* the IPv4 header, as 4.4BSD declares it (little-endian i960) */
#include <netinet/in.h>
#include <netinet/in_systm.h>

#define IPVERSION	4

struct ip {
	unsigned char	ip_hl:4, ip_v:4;
	unsigned char	ip_tos;
	unsigned short	ip_len;
	unsigned short	ip_id;
	unsigned short	ip_off;
	unsigned char	ip_ttl;
	unsigned char	ip_p;
	unsigned short	ip_sum;
	struct in_addr	ip_src, ip_dst;
};

#define IP_RF		0x8000
#define IP_DF		0x4000
#define IP_MF		0x2000
#define IP_OFFMASK	0x1fff
#define IP_MAXPACKET	65535
#define MAXTTL		255

struct iphdr {
	unsigned char	ihl:4, version:4;
	unsigned char	tos;
	unsigned short	tot_len;
	unsigned short	id;
	unsigned short	frag_off;
	unsigned char	ttl;
	unsigned char	protocol;
	unsigned short	check;
	unsigned long	saddr, daddr;
};
#endif
