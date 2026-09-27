/*
 * rshd - a minimal remote shell daemon for netOS, after BSD rshd(8).
 * Ported from tenox's minirshd; adapted to netOS, which has vfork but no fork.
 *
 *   rshd [port]        default 514; run a command sent by rsh/rcmd
 *
 * One client at a time.  The rcmd protocol is: a NUL-terminated stderr port
 * ("0" = fold stderr into the connection), remote user, local user, command.
 * The daemon reads all of that, then vforks a "/bin/sh -c command" whose
 * stdin/stdout are the connection (stderr the secondary channel, if any).
 *
 * No authentication - anyone who can reach the port can run a command.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define RSH_PORT 514
#define MAXCMD   4096
#define SHELL    "/bin/sh"

/* Read a NUL-terminated string from the socket; returns 0 on success. */
static int read_str(int s, char *buf, int max)
{
	int i, n;

	for (i = 0; i < max; i++) {
		n = read(s, &buf[i], 1);
		if (n <= 0) return -1;
		if (!buf[i]) return 0;
	}
	return -1;
}

/* Send a one-byte error indication and the message, as rsh expects. */
static void reject(int s, const char *msg)
{
	char buf[128];

	buf[0] = 1;
	strncpy(buf + 1, msg, sizeof buf - 3);
	buf[sizeof buf - 2] = 0;
	strcat(buf, "\n");
	write(s, buf, strlen(buf));
	fprintf(stderr, "rshd: %s\n", msg);
}

/* Open the secondary stderr channel back to the client. */
static int stderr_sock(struct sockaddr_in *peer, int port)
{
	struct sockaddr_in a;
	int s;

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) return -1;
	a = *peer;
	a.sin_port = htons(port);
	if (connect(s, (struct sockaddr *) &a, sizeof a) < 0) {
		close(s);
		return -1;
	}
	return s;
}

static void run(int sock, struct sockaddr_in *peer)
{
	static char cmd[MAXCMD];
	char portstr[8], ruser[64], luser[64];
	int errport, errfd, status, pid;

	if (read_str(sock, portstr, sizeof portstr)) { reject(sock, "bad stderr port"); return; }
	errport = atoi(portstr);
	if (read_str(sock, ruser, sizeof ruser)) { reject(sock, "bad remote user"); return; }
	if (read_str(sock, luser, sizeof luser)) { reject(sock, "bad local user"); return; }
	if (read_str(sock, cmd, sizeof cmd)) { reject(sock, "command too long"); return; }

	fprintf(stderr, "rshd: %s@%s cmd=\"%s\"\n", ruser, inet_ntoa(peer->sin_addr), cmd);

	errfd = sock;
	if (errport > 0 && (errfd = stderr_sock(peer, errport)) < 0) {
		reject(sock, "can't connect stderr");
		return;
	}

	write(sock, "", 1);			/* success: one zero byte */

	pid = vfork();
	if (pid < 0) { reject(sock, "vfork failed"); if (errfd != sock) close(errfd); return; }
	if (pid == 0) {				/* child: only fd setup + exec */
		dup2(sock, 0);
		dup2(sock, 1);
		dup2(errfd, 2);
		if (sock > 2) close(sock);
		if (errfd > 2 && errfd != sock) close(errfd);
		execl(SHELL, "sh", "-c", cmd, (char *) 0);
		_exit(127);
	}
	if (errfd != sock) close(errfd);
	wait(&status);
	fprintf(stderr, "rshd: command exited %d\n", WEXITSTATUS(status));
}

int main(int argc, char **argv)
{
	struct sockaddr_in addr;
	int lsock, csock, on = 1, port = RSH_PORT;
	socklen_t len;

	if (argc > 1) port = atoi(argv[1]);
	if (argc > 2 || port <= 0 || port > 65535) {
		fprintf(stderr, "usage: rshd [port]\n");
		return 1;
	}

	if ((lsock = socket(AF_INET, SOCK_STREAM, 0)) < 0) { perror("socket"); return 1; }
	setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
	memset(&addr, 0, sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);
	if (bind(lsock, (struct sockaddr *) &addr, sizeof addr) < 0) { perror("bind"); return 1; }
	if (listen(lsock, 5) < 0) { perror("listen"); return 1; }
	fprintf(stderr, "rshd: listening on port %d\n", port);

	for (;;) {
		len = sizeof addr;
		if ((csock = accept(lsock, (struct sockaddr *) &addr, &len)) < 0) {
			perror("accept");
			sleep(1);
			continue;
		}
		fprintf(stderr, "rshd: connection from %s:%d\n",
			inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
		run(csock, &addr);
		shutdown(csock, 2);
		close(csock);
	}
	return 0;
}
