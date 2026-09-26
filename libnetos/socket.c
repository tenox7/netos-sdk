#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

extern struct hostent *netos_gethostbyname(const char *);
extern struct hostent *netos_gethostbyaddr(const char *, int, int);
extern void netos_free(void *);

int h_errno;

int recv(int s, void *b, int n, int f)
{
	return f ? recvfrom(s, b, n, f, 0, 0) : read(s, b, n);
}

int send(int s, const void *b, int n, int f)
{
	return f ? sendto(s, b, n, f, 0, 0) : write(s, b, n);
}

/* netOS hands back a malloc'd hostent; copy it to static storage and give
   the original back, as the shipped netOS applications do */
static struct hostent *keephost(struct hostent *hp)
{
	static struct hostent h;
	static char name[256];
	static struct in_addr addr;
	static char *list[2];

	h_errno = HOST_NOT_FOUND;
	if (!hp) return 0;
	if (!hp->h_addr_list || !hp->h_addr_list[0]) {
		netos_free(hp);
		return 0;
	}
	strncpy(name, hp->h_name ? hp->h_name : "", sizeof name - 1);
	memcpy(&addr, hp->h_addr_list[0], sizeof addr);
	h.h_addrtype = hp->h_addrtype;
	netos_free(hp);
	list[0] = (char *) &addr;
	h.h_name = name;
	h.h_aliases = list + 1;
	h.h_addr_list = list;
	h.h_length = sizeof addr;
	h_errno = 0;
	return &h;
}

struct hostent *gethostbyname(const char *name)
{
	return keephost(netos_gethostbyname(name));
}

struct hostent *gethostbyaddr(const char *a, int len, int type)
{
	return keephost(netos_gethostbyaddr(a, len, type));
}

int inet_aton(const char *s, struct in_addr *a)
{
	in_addr_t v = 0;
	int i, n;

	for (i = 0; i < 4; i++) {
		if (*s < '0' || *s > '9') return 0;
		for (n = 0; *s >= '0' && *s <= '9'; s++)
			if ((n = n * 10 + *s - '0') > 255) return 0;
		v = v << 8 | n;
		if (i < 3 && *s++ != '.') return 0;
	}
	if (*s) return 0;
	if (a) a->s_addr = htonl(v);
	return 1;
}

in_addr_t inet_addr(const char *s)
{
	struct in_addr a;

	return inet_aton(s, &a) ? a.s_addr : INADDR_NONE;
}

static char *dec(char *p, unsigned n)
{
	if (n >= 10) p = dec(p, n / 10);
	*p++ = (char) ('0' + n % 10);
	return p;
}

char *inet_ntoa(struct in_addr a)
{
	static char buf[16];
	unsigned char *b = (unsigned char *) &a.s_addr;
	char *p = buf;
	int i;

	for (i = 0; i < 4; i++) {
		p = dec(p, b[i]);
		*p++ = i < 3 ? '.' : 0;
	}
	return buf;
}
