/*
 * nettest - exercise the netOS socket layer, printing every call.
 *
 *   nettest [-u] [port]              echo server, TCP (or UDP), default 7777
 *   nettest [-u] host port [text]    client: send text, print the reply
 *
 * Send "quit" to a server to stop it.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

static int show(const char *what, int r)
{
	if (r < 0) printf("%s = %d, %s (errno %d)\n", what, r, strerror(errno), errno);
	else printf("%s = %d\n", what, r);
	return r;
}

static char *addrstr(struct sockaddr_in *sin)
{
	static char buf[300];
	struct hostent *hp;

	hp = gethostbyaddr((char *) &sin->sin_addr, sizeof sin->sin_addr, AF_INET);
	sprintf(buf, "%s:%d (%s)", inet_ntoa(sin->sin_addr), ntohs(sin->sin_port),
		hp ? hp->h_name : "no name");
	return buf;
}

static int readable(int s, int secs)
{
	fd_set r;
	struct timeval tv;

	FD_ZERO(&r);
	FD_SET(s, &r);
	tv.tv_sec = secs;
	tv.tv_usec = 0;
	return select(s + 1, &r, 0, 0, &tv);
}

static void services(void)
{
	static const char *name[] = { "exec", "login", "shell", "telnet", 0 };
	struct servent *sp;
	int i;

	for (i = 0; name[i]; i++) {
		sp = getservbyname(name[i], "tcp");
		printf("getservbyname %s/tcp = %d\n", name[i], sp ? ntohs(sp->s_port) : -1);
	}
}

static int listener(int type, int port)
{
	struct sockaddr_in sin;
	int s, on = 1;

	if (show("socket", s = socket(AF_INET, type, 0)) < 0) return -1;
	show("setsockopt SO_REUSEADDR", setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on));
	memset(&sin, 0, sizeof sin);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	if (show("bind", bind(s, (struct sockaddr *) &sin, sizeof sin)) < 0) return -1;
	if (type == SOCK_STREAM && show("listen", listen(s, 5)) < 0) return -1;
	return s;
}

static int session(int c)
{
	static const char hi[] = "netOS nettest echo, send quit to stop\r\n";
	struct sockaddr_in a;
	socklen_t len = sizeof a;
	char buf[512];
	int n;

	if (show("getpeername", getpeername(c, (struct sockaddr *) &a, &len)) == 0)
		printf("  peer %s\n", addrstr(&a));
	len = sizeof a;
	if (show("getsockname", getsockname(c, (struct sockaddr *) &a, &len)) == 0)
		printf("  local %s\n", addrstr(&a));
	show("write", write(c, hi, sizeof hi - 1));
	while ((n = read(c, buf, sizeof buf)) > 0) {
		if (n >= 4 && !strncmp(buf, "quit", 4)) return 1;
		printf("echo %d bytes\n", n);
		write(c, buf, n);
	}
	show("read", n);
	return 0;
}

static int tcp_server(int port)
{
	struct sockaddr_in a;
	socklen_t len;
	int s, c, quit = 0;

	services();
	if ((s = listener(SOCK_STREAM, port)) < 0) return -1;
	while (!quit) {
		printf("waiting on tcp port %d\n", port);
		len = sizeof a;
		if (show("accept", c = accept(s, (struct sockaddr *) &a, &len)) < 0) return -1;
		printf("  from %s\n", addrstr(&a));
		quit = session(c);
		show("shutdown", shutdown(c, 2));
		show("close", close(c));
	}
	close(s);
	return 0;
}

static int udp_server(int port)
{
	struct sockaddr_in a;
	socklen_t len;
	char buf[512];
	int s, n;

	if ((s = listener(SOCK_DGRAM, port)) < 0) return -1;
	for (;;) {
		printf("waiting on udp port %d\n", port);
		len = sizeof a;
		if (show("recvfrom", n = recvfrom(s, buf, sizeof buf, 0, (struct sockaddr *) &a, &len)) < 0)
			return -1;
		printf("  from %s\n", addrstr(&a));
		if (n >= 4 && !strncmp(buf, "quit", 4)) break;
		show("sendto", sendto(s, buf, n, 0, (struct sockaddr *) &a, len));
	}
	close(s);
	return 0;
}

static int resolve(const char *host, int port, struct sockaddr_in *sin)
{
	struct hostent *hp;

	memset(sin, 0, sizeof *sin);
	sin->sin_family = AF_INET;
	sin->sin_port = htons(port);
	if (inet_aton(host, &sin->sin_addr)) return 0;
	if (!(hp = gethostbyname(host))) {
		printf("gethostbyname %s: unknown host\n", host);
		return -1;
	}
	memcpy(&sin->sin_addr, hp->h_addr, sizeof sin->sin_addr);
	printf("gethostbyname %s = %s (%s)\n", host, inet_ntoa(sin->sin_addr), hp->h_name);
	return 0;
}

static void drain(int s, int secs)
{
	char buf[512];
	int n;

	while (readable(s, secs) > 0 && (n = read(s, buf, sizeof buf)) > 0) {
		fwrite(buf, 1, n, stdout);
		fflush(stdout);
	}
}

static int tcp_client(struct sockaddr_in *sin, const char *text)
{
	static const char head[] = "HEAD / HTTP/1.0\r\n\r\n";
	int s;

	if (show("socket", s = socket(AF_INET, SOCK_STREAM, 0)) < 0) return -1;
	if (show("connect", connect(s, (struct sockaddr *) sin, sizeof *sin)) < 0) return -1;
	printf("  to %s\n", addrstr(sin));
	if (text) {
		write(s, text, strlen(text));
		write(s, "\r\n", 2);
	} else if (ntohs(sin->sin_port) == 80) {
		write(s, head, sizeof head - 1);
	}
	drain(s, 5);
	close(s);
	return 0;
}

static int udp_client(struct sockaddr_in *sin, const char *text)
{
	struct sockaddr_in a;
	socklen_t len = sizeof a;
	char buf[512];
	int s, n;

	if (!text) text = "hello from netOS";
	if (show("socket", s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) return -1;
	if (show("sendto", sendto(s, text, strlen(text), 0, (struct sockaddr *) sin, sizeof *sin)) < 0)
		return -1;
	if (show("select", readable(s, 3)) <= 0) return -1;
	if (show("recvfrom", n = recvfrom(s, buf, sizeof buf - 1, 0, (struct sockaddr *) &a, &len)) < 0)
		return -1;
	buf[n] = 0;
	printf("  from %s: %s\n", addrstr(&a), buf);
	close(s);
	return 0;
}

int main(int argc, char **argv)
{
	struct sockaddr_in sin;
	char *text;
	int udp = 0, port;

	if (argc > 1 && !strcmp(argv[1], "-u")) {
		udp = 1;
		argc--, argv++;
	}
	if (argc < 3) {
		port = argc > 1 ? atoi(argv[1]) : 7777;
		return (udp ? udp_server(port) : tcp_server(port)) < 0;
	}
	if (resolve(argv[1], atoi(argv[2]), &sin) < 0) return 1;
	text = argc > 3 ? argv[3] : 0;
	return (udp ? udp_client(&sin, text) : tcp_client(&sin, text)) < 0;
}
