#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(void)
{
	char *v[64];
	int i, n = 0, hr, mn;
	for (i = 0; i < 64; i++) {
		if (!(v[i] = malloc(4096))) break;
		memset(v[i], 0x41, 4096);
		n++;
	}
	printf("allocated %d x 4096 = %d bytes from the netOS heap\n", n, n * 4096);
	for (i = 0; i < n; i++) free(v[i]);
	sscanf("13:37", "%d:%d", &hr, &mn);
	printf("sscanf gave %d and %d\n", hr, mn);
	printf("getenv(DISPLAY) = %s\n", getenv("DISPLAY") ? getenv("DISPLAY") : "(unset)");
	return 0;
}
