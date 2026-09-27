/*
 * telnetd - a minimal telnet server for netOS.
 *
 *   telnetd [port]      default 23
 *
 * One session at a time; later callers are told it is busy.  Like the
 * kernel's own "pty" program, it runs the netOS shell on one end of a
 * socketpair and supplies the line discipline itself: echo, BS/DEL erase,
 * ^U kill, ^C interrupt, ^D end of input, CR as newline, LF -> CRLF out.
 * Programs switch it to raw or no-echo with the netOS pty flags, as they do
 * the kernel's pty.
 *
 * No authentication - anyone who can reach the port gets a shell.
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>

#define SHELL		"/bin/sh"
#define TGET		0x6608		/* ioctl: the pty flags a program set */
#define TRAW		0x4000
#define TNOECHO		0x8000

enum { DATA, CMD, OPT, SUB, SUBIAC };

struct sess {
	int net, sh, pid;
	int state, verb, cr, len, fl, sblen, rows, cols;
	char line[512], term[32];
	unsigned char sb[64];
};

static void put(int fd, const void *p, int n)
{
	write(fd, p, n);
}

static void option(int net, int verb, int opt)
{
	unsigned char b[3];

	b[0] = IAC;
	b[1] = verb;
	b[2] = opt;
	put(net, b, 3);
}

/* keep echo, suppress-go-ahead, terminal type and window size; refuse the rest */
static void negotiate(struct sess *s, int opt)
{
	static const unsigned char ttype[] = { IAC, SB, TELOPT_TTYPE, TELQUAL_SEND, IAC, SE };

	if (s->verb == WILL && opt == TELOPT_TTYPE) put(s->net, ttype, sizeof ttype);
	else if (s->verb == WILL && opt != TELOPT_NAWS) option(s->net, opt == TELOPT_SGA ? DO : DONT, opt);
	if (s->verb == DO && opt != TELOPT_ECHO && opt != TELOPT_SGA) option(s->net, WONT, opt);
}

static int flags(struct sess *s)
{
	int fl = 0;

	ioctl(s->sh, TGET, &fl);
	return fl;
}

static void echo(struct sess *s, const void *p, int n)
{
	if (!(s->fl & TNOECHO)) put(s->net, p, n);
}

static void erase(struct sess *s, int n)
{
	while (n-- > 0 && s->len) {
		s->len--;
		echo(s, "\b \b", 3);
	}
}

static void key(struct sess *s, int c)
{
	char ch = c;

	if (s->cr && (c == '\n' || c == 0)) {
		s->cr = 0;
		return;
	}
	s->cr = c == '\r';
	if (s->fl & TRAW) {
		put(s->sh, &ch, 1);
		echo(s, &ch, 1);
		return;
	}
	switch (c) {
	case '\r':
	case '\n':
		echo(s, "\r\n", 2);
		s->line[s->len++] = '\n';
		put(s->sh, s->line, s->len);
		s->len = 0;
		return;
	case 8:
	case 127:
		erase(s, 1);
		return;
	case 21:
		erase(s, s->len);
		return;
	case 3:
		s->len = 0;
		put(s->net, "^C\r\n", 4);
		kill(s->pid, SIGINT);
		put(s->sh, "\n", 1);		/* netOS sh only re-prompts on input */
		return;
	case 4:
		if (!s->len) shutdown(s->sh, 1);
		return;
	}
	if ((c < ' ' && c != '\t') || s->len >= (int) sizeof s->line - 1) return;
	s->line[s->len++] = ch;
	echo(s, &ch, 1);
}

/* window size and terminal type, for the shell's environment */
static void sub(struct sess *s)
{
	unsigned char *b = s->sb;
	int i;

	if (s->sblen >= 5 && b[0] == TELOPT_NAWS) {
		s->cols = b[1] << 8 | b[2];
		s->rows = b[3] << 8 | b[4];
	}
	if (s->sblen < 3 || b[0] != TELOPT_TTYPE || b[1] != TELQUAL_IS) return;
	for (i = 0; i < s->sblen - 2 && i < (int) sizeof s->term - 1; i++)
		s->term[i] = tolower(b[i + 2]);
	s->term[i] = 0;
}

static void net_in(struct sess *s, const unsigned char *p, int n)
{
	int c;

	s->fl = flags(s);
	while (n-- > 0) {
		c = *p++;
		switch (s->state) {
		case DATA:
			if (c == IAC) s->state = CMD;
			else key(s, c);
			break;
		case CMD:
			s->state = DATA;
			if (c == IAC) key(s, c);
			else if (c == SB) {
				s->sblen = 0;
				s->state = SUB;
			} else if (c >= WILL && c <= DONT) {
				s->verb = c;
				s->state = OPT;
			}
			break;
		case OPT:
			negotiate(s, c);
			s->state = DATA;
			break;
		case SUB:
			if (c == IAC) s->state = SUBIAC;
			else if (s->sblen < (int) sizeof s->sb) s->sb[s->sblen++] = c;
			break;
		case SUBIAC:
			s->state = c == SE ? DATA : SUB;
			if (c == SE) sub(s);
			else if (c == IAC && s->sblen < (int) sizeof s->sb) s->sb[s->sblen++] = c;
			break;
		}
	}
}

static void sh_out(struct sess *s, const unsigned char *p, int n)
{
	unsigned char b[1024];
	int i, k = 0, raw = flags(s) & TRAW;

	for (i = 0; i < n; i++) {
		if (p[i] == '\n' && !raw) b[k++] = '\r';
		if (p[i] == IAC) b[k++] = IAC;
		b[k++] = p[i];
	}
	put(s->net, b, k);
}

static int spawn(int *fd, int net, int lsock)
{
	int sv[2], pid;

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0) return -1;
	pid = vfork();
	if (pid == 0) {
		dup2(sv[1], 0);
		dup2(sv[1], 1);
		dup2(sv[1], 2);
		close(sv[0]);
		if (sv[1] > 2) close(sv[1]);
		if (net > 2) close(net);
		if (lsock > 2) close(lsock);
		execl(SHELL, "sh", (char *) 0);
		_exit(127);
	}
	close(sv[1]);
	if (pid < 0) {
		close(sv[0]);
		return -1;
	}
	*fd = sv[0];
	return pid;
}

static void busy(int lsock)
{
	static const char msg[] = "telnetd: busy, one session at a time\r\n";
	struct sockaddr_in a;
	socklen_t len = sizeof a;
	int c;

	if ((c = accept(lsock, (struct sockaddr *) &a, &len)) < 0) return;
	put(c, msg, sizeof msg - 1);
	close(c);
}

/* give the client a second to report its terminal before the shell starts */
static void settle(struct sess *s)
{
	unsigned char buf[256];
	char v[16];
	struct timeval tv;
	fd_set r;
	int n;

	tv.tv_sec = 1;
	tv.tv_usec = 0;
	while (!(s->term[0] && s->cols)) {
		FD_ZERO(&r);
		FD_SET(s->net, &r);
		if (select(s->net + 1, &r, 0, 0, &tv) <= 0 || (n = read(s->net, buf, sizeof buf)) <= 0) break;
		net_in(s, buf, n);
	}
	setenv("TERM", s->term[0] ? s->term : "vt220", 1);
	unsetenv("COLUMNS");
	unsetenv("LINES");
	if (s->cols && s->rows) {
		sprintf(v, "%d", s->cols);
		setenv("COLUMNS", v, 1);
		sprintf(v, "%d", s->rows);
		setenv("LINES", v, 1);
	}
}

static void session(int net, int lsock)
{
	static const unsigned char hello[] = {
		IAC, WILL, TELOPT_ECHO, IAC, WILL, TELOPT_SGA, IAC, DO, TELOPT_TTYPE, IAC, DO, TELOPT_NAWS
	};
	static const char banner[] = "netOS telnetd - exit or ^D to quit\r\n";
	struct sess s;
	unsigned char buf[512];
	fd_set r;
	int n, st, max;

	memset(&s, 0, sizeof s);
	s.net = net;
	s.sh = -1;
	put(net, hello, sizeof hello);
	settle(&s);
	if ((s.pid = spawn(&s.sh, net, lsock)) < 0) {
		put(net, "telnetd: can't start shell\r\n", 28);
		return;
	}
	put(net, banner, sizeof banner - 1);
	max = net > s.sh ? net : s.sh;
	if (lsock > max) max = lsock;
	for (;;) {
		FD_ZERO(&r);
		FD_SET(net, &r);
		FD_SET(s.sh, &r);
		FD_SET(lsock, &r);
		if (select(max + 1, &r, 0, 0, 0) < 0) {
			if (errno == EINTR) continue;
			break;
		}
		if (FD_ISSET(lsock, &r)) busy(lsock);
		if (FD_ISSET(s.sh, &r)) {
			if ((n = read(s.sh, buf, sizeof buf)) <= 0) break;
			sh_out(&s, buf, n);
		}
		if (FD_ISSET(net, &r)) {
			if ((n = read(net, buf, sizeof buf)) <= 0) {
				kill(s.pid, SIGKILL);
				break;
			}
			net_in(&s, buf, n);
		}
	}
	close(s.sh);
	while ((n = wait(&st)) != s.pid && n != -1)
		;
}

int main(int argc, char **argv)
{
	struct sockaddr_in a;
	socklen_t len;
	int lsock, net, on = 1, port = argc > 1 ? atoi(argv[1]) : 23;

	if (argc > 2 || port <= 0 || port > 65535) {
		fprintf(stderr, "usage: telnetd [port]\n");
		return 1;
	}
	if ((lsock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return 1;
	}
	setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
	memset(&a, 0, sizeof a);
	a.sin_family = AF_INET;
	a.sin_port = htons(port);
	if (bind(lsock, (struct sockaddr *) &a, sizeof a) < 0) {
		perror("bind");
		return 1;
	}
	if (listen(lsock, 5) < 0) {
		perror("listen");
		return 1;
	}
	fprintf(stderr, "telnetd: listening on port %d\n", port);
	for (;;) {
		len = sizeof a;
		if ((net = accept(lsock, (struct sockaddr *) &a, &len)) < 0) {
			perror("accept");
			sleep(1);
			continue;
		}
		fprintf(stderr, "telnetd: session from %s\n", inet_ntoa(a.sin_addr));
		session(net, lsock);
		close(net);
		fprintf(stderr, "telnetd: session ended\n");
	}
}
