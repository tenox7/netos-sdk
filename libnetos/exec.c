/* netOS has vfork but no fork, and wait but no waitpid.  Its wait status is
   the bare exit code (or signal number), given back here as an exit code. */
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

extern int netos_execve(const char *, char *const *, char *const *);
extern int netos_wait(int *);
extern void netos_exit(int), netos_vexit(int);
extern int __netos_vforked;

void _exit(int code)
{
	if (__netos_vforked) netos_vexit(code);
	netos_exit(code);
	for (;;)
		;
}

int fork(void)
{
	errno = ENOSYS;
	return -1;
}

int execve(const char *path, char *const argv[], char *const envp[])
{
	return netos_execve(path, argv, envp);
}

int _execve(const char *path, char *const argv[], char *const envp[])
{
	return netos_execve(path, argv, envp);
}

pid_t wait(int *st)
{
	int s;
	pid_t r = netos_wait(&s);

	if (r != -1 && st) *st = (s & 0xff) << 8;
	return r;
}

pid_t waitpid(pid_t pid, int *st, int opt)
{
	pid_t r;

	if (opt & WNOHANG) return 0;
	while ((r = wait(st)) != -1 && pid > 0 && r != pid)
		;
	return r;
}

int system(const char *cmd)
{
	int pid, st;

	if (!cmd) return 1;
	if ((pid = vfork()) < 0) return -1;
	if (!pid) {
		execl("/bin/sh", "sh", "-c", cmd, (char *) 0);
		_exit(127);
	}
	return waitpid(pid, &st, 0) < 0 ? -1 : st;
}
