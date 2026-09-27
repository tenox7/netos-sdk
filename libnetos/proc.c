/* netOS has no users or process groups, and its kernel signals never
   reach newlib's handlers; raise() is newlib's */
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>

pid_t getppid(void) { return 1; }
uid_t getuid(void) { return 0; }
uid_t geteuid(void) { return 0; }
gid_t getgid(void) { return 0; }
gid_t getegid(void) { return 0; }
int setuid(uid_t u) { return 0; }
int setgid(gid_t g) { return 0; }
int getgroups(int n, gid_t *g) { return 0; }
pid_t getpgrp(void) { return getpid(); }
pid_t setsid(void) { return getpid(); }
int setpgid(pid_t p, pid_t g) { return 0; }
int killpg(pid_t g, int sig) { return kill(g, sig); }
unsigned alarm(unsigned s) { return 0; }

extern int sigblock(int), sigsetmask(int);

int sigprocmask(int how, const sigset_t *set, sigset_t *old)
{
	int m = sigblock(0);

	if (old) *old = m;
	if (!set) return 0;
	if (how == SIG_BLOCK) m |= *set;
	else if (how == SIG_UNBLOCK) m &= ~*set;
	else m = *set;
	sigsetmask(m);
	return 0;
}

int sigaction(int sig, const struct sigaction *a, struct sigaction *old)
{
	_sig_func_ptr h = a ? signal(sig, a->sa_handler) : signal(sig, SIG_IGN);

	if (h == SIG_ERR) return -1;
	if (!a) signal(sig, h);
	if (old) {
		memset(old, 0, sizeof *old);
		old->sa_handler = h;
	}
	return 0;
}

int sigsuspend(const sigset_t *m) { return pause(); }

int pause(void)
{
	select(0, 0, 0, 0, 0);
	errno = EINTR;
	return -1;
}


pid_t getpgid(pid_t p) { return p ? p : getpid(); }
pid_t getsid(pid_t p) { return p ? p : getpid(); }
int sigemptyset(sigset_t *s) { *s = 0; return 0; }
int sigfillset(sigset_t *s) { *s = ~0; return 0; }
int sigaddset(sigset_t *s, int n) { *s |= 1UL << (n - 1); return 0; }
int sigdelset(sigset_t *s, int n) { *s &= ~(1UL << (n - 1)); return 0; }
int sigismember(const sigset_t *s, int n) { return (*s >> (n - 1)) & 1; }
int sigpending(sigset_t *s) { *s = 0; return 0; }
int issetugid(void) { return 0; }
