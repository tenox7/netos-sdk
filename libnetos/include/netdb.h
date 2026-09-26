#ifndef _NETDB_H
#define _NETDB_H

struct hostent {
	char	*h_name;
	char	**h_aliases;
	int	h_addrtype;
	int	h_length;
	char	**h_addr_list;
};
#define h_addr	h_addr_list[0]

struct servent {
	char	*s_name;
	char	**s_aliases;
	int	s_port;		/* network byte order */
	char	*s_proto;
};

#define HOST_NOT_FOUND	1

extern int h_errno;

struct hostent *gethostbyname(const char *);
struct hostent *gethostbyaddr(const char *, int, int);
struct servent *getservbyname(const char *, const char *);
#endif
