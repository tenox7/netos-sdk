#include <unistd.h>
#include <stdarg.h>

extern int execv(const char *, char *const *);

/* execl(path, arg0, arg1, ..., NULL) -> execv.  Safe after vfork because it
   never returns on success and touches only its own frame on failure. */
int execl(const char *path, const char *arg0, ...)
{
	char *argv[64];
	va_list ap;
	int n = 0;

	argv[n++] = (char *) arg0;
	va_start(ap, arg0);
	while (n < 63 && (argv[n] = va_arg(ap, char *)) != 0)
		n++;
	va_end(ap);
	argv[n] = 0;
	return execv(path, argv);
}
