#ifndef _NETINET_IP_ICMP_H
#define _NETINET_IP_ICMP_H
/* ICMP, as 4.4BSD declares it */
#include <netinet/ip.h>

struct icmp {
	unsigned char	icmp_type;
	unsigned char	icmp_code;
	unsigned short	icmp_cksum;
	union {
		unsigned char	ih_pptr;
		struct in_addr	ih_gwaddr;
		struct { unsigned short icd_id, icd_seq; } ih_idseq;
		unsigned long	ih_void;
	} icmp_hun;
#define icmp_pptr	icmp_hun.ih_pptr
#define icmp_gwaddr	icmp_hun.ih_gwaddr
#define icmp_id		icmp_hun.ih_idseq.icd_id
#define icmp_seq	icmp_hun.ih_idseq.icd_seq
#define icmp_void	icmp_hun.ih_void
	union {
		struct { n_time its_otime, its_rtime, its_ttime; } id_ts;
		struct { struct ip idi_ip; } id_ip;
		unsigned long	id_mask;
		char		id_data[1];
	} icmp_dun;
#define icmp_otime	icmp_dun.id_ts.its_otime
#define icmp_rtime	icmp_dun.id_ts.its_rtime
#define icmp_ttime	icmp_dun.id_ts.its_ttime
#define icmp_ip		icmp_dun.id_ip.idi_ip
#define icmp_mask	icmp_dun.id_mask
#define icmp_data	icmp_dun.id_data
};

#define ICMP_MINLEN		8
#define ICMP_ECHOREPLY		0
#define ICMP_UNREACH		3
#define ICMP_SOURCEQUENCH	4
#define ICMP_REDIRECT		5
#define ICMP_ECHO		8
#define ICMP_TIMXCEED		11
#define ICMP_PARAMPROB		12
#define ICMP_TSTAMP		13
#define ICMP_TSTAMPREPLY	14
#define ICMP_IREQ		15
#define ICMP_IREQREPLY		16
#define ICMP_MASKREQ		17
#define ICMP_MASKREPLY		18

/* the Linux names */
#define ICMP_DEST_UNREACH	ICMP_UNREACH
#define ICMP_SOURCE_QUENCH	ICMP_SOURCEQUENCH
#define ICMP_TIME_EXCEEDED	ICMP_TIMXCEED
#define ICMP_PARAMETERPROB	ICMP_PARAMPROB
#define ICMP_TIMESTAMP		ICMP_TSTAMP
#define ICMP_TIMESTAMPREPLY	ICMP_TSTAMPREPLY
#define ICMP_INFO_REQUEST	ICMP_IREQ
#define ICMP_INFO_REPLY		ICMP_IREQREPLY
#define ICMP_ADDRESS		ICMP_MASKREQ
#define ICMP_ADDRESSREPLY	ICMP_MASKREPLY
#endif
