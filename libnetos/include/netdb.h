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

struct protoent {
	char	*p_name;
	char	**p_aliases;
	int	p_proto;
};

struct netent {
	char		*n_name;
	char		**n_aliases;
	int		n_addrtype;
	unsigned long	n_net;
};

#define HOST_NOT_FOUND	1
#define TRY_AGAIN	2
#define NO_RECOVERY	3
#define NO_DATA		4
#define NO_ADDRESS	NO_DATA

extern int h_errno;

struct hostent	*gethostbyname(const char *);
struct hostent	*gethostbyaddr(const char *, int, int);
struct servent	*getservbyname(const char *, const char *);
struct servent	*getservbyport(int, const char *);
struct protoent	*getprotobyname(const char *);
struct protoent	*getprotobynumber(int);
struct netent	*getnetbyname(const char *);
struct netent	*getnetbyaddr(unsigned long, int);
void		herror(const char *);
const char	*hstrerror(int);
#endif
