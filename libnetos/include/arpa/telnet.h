#ifndef _ARPA_TELNET_H
#define _ARPA_TELNET_H
/* the telnet protocol, RFC 854 and friends */
#define IAC	255
#define DONT	254
#define DO	253
#define WONT	252
#define WILL	251
#define SB	250
#define GA	249
#define EL	248
#define EC	247
#define AYT	246
#define AO	245
#define IP	244
#define BREAK	243
#define DM	242
#define NOP	241
#define SE	240
#define EOR	239
#define SYNCH	242

#define TELOPT_BINARY	0
#define TELOPT_ECHO	1
#define TELOPT_SGA	3
#define TELOPT_STATUS	5
#define TELOPT_TM	6
#define TELOPT_TTYPE	24
#define TELOPT_EOR	25
#define TELOPT_NAWS	31
#define TELOPT_TSPEED	32
#define TELOPT_LFLOW	33
#define TELOPT_LINEMODE	34
#define TELOPT_XDISPLOC	35
#define TELOPT_ENVIRON	36
#define TELOPT_NEW_ENVIRON 39

#define TELQUAL_IS	0
#define TELQUAL_SEND	1
#define TELQUAL_INFO	2

#define NEW_ENV_VAR	0
#define NEW_ENV_VALUE	1
#define ENV_ESC		2
#define ENV_USERVAR	3
#endif
