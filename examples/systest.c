#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netos.h>

int main(void)
{
	char buf[256], sb[128];
	char *p;
	int n;

	printf("--- netOS kernel libc, called directly ---\n");
	printf("strlen  : %d\n", netos_strlen("hello netOS"));
	printf("strcmp  : %d %d\n", netos_strcmp("abc", "abc"), netos_strcmp("abc", "abd"));
	netos_strcpy(buf, "copied by the kernel");
	printf("strcpy  : %s\n", buf);
	netos_sprintf(buf, "%s=%d", "answer", 42);
	printf("sprintf : %s\n", buf);
	printf("getenv  : DISPLAY=%s\n", netos_getenv("DISPLAY") ? netos_getenv("DISPLAY") : "(unset)");
	p = netos_malloc(1024);
	printf("malloc  : %s\n", p ? "ok" : "failed");
	netos_free(p);
	printf("stat /  : %d\n", netos_stat("/", sb));
	n = 0;
	netos_sscanf("7 42", "%d %d", &n, &n);
	printf("sscanf  : %d\n", n);
	return 0;
}
