#ifndef _SYS_WAIT_H
#define _SYS_WAIT_H
int wait(int *);
/* netOS has no waitpid; wait() returns the pid that exited */
#define waitpid(pid, sp, opt) wait(sp)
#define WIFEXITED(s)	(((s) & 0x7f) == 0)
#define WEXITSTATUS(s)	(((s) >> 8) & 0xff)
#define WIFSIGNALED(s)	(((s) & 0x7f) != 0 && ((s) & 0x7f) != 0x7f)
#define WTERMSIG(s)	((s) & 0x7f)
#endif
