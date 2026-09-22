#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	char *p;
	time_t t;
	struct tm *lt;
	int i;

	printf("netOS SDK demo, argc=%d\n", argc);
	for (i = 0; i < argc; i++)
		printf("  argv[%d] = \"%s\"\n", i, argv[i]);

	p = malloc(64);
	strcpy(p, "malloc/strcpy/sprintf all work");
	printf("heap: %s (len %d)\n", p, (int) strlen(p));
	free(p);

	printf("formats: |%5d|%-5d|%05d|%x|%X|%o|%c|%s|%p|%%|\n",
	       42, 42, 42, 48879, 48879, 8, 'Z', "str", (void *) 0x1234);

	t = time(0);
	lt = localtime(&t);
	printf("time: %04d-%02d-%02d %02d:%02d:%02d\n",
	       lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday,
	       lt->tm_hour, lt->tm_min, lt->tm_sec);

	printf("sleeping 1s...\n");
	sleep(1);
	puts("done.");
	return 0;
}
