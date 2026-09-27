/* netOS has no getty to set TERM; its Console Window is a VT320 */
#include <stdlib.h>
#include <string.h>

extern char **environ;

void __netos_init(void)
{
	static char term[] = "TERM=vt220";
	char **n;
	int i;

	for (i = 0; environ && environ[i]; i++)
		if (!strncmp(environ[i], "TERM=", 5)) return;
	if (!(n = malloc((i + 2) * sizeof *n))) return;
	if (i) memcpy(n, environ, i * sizeof *n);
	n[i] = term;
	n[i + 1] = 0;
	environ = n;
}
