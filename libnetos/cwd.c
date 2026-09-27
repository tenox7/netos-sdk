#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* ".." stops at a mount's root, so the mount point is found by name in "/" */
static int lookup(const char *dir, struct stat *want, char *name)
{
	char p[PATH_MAX];
	struct dirent *e;
	struct stat s;
	DIR *d = opendir(dir);

	if (!d) return -1;
	while ((e = readdir(d))) {
		if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) continue;
		snprintf(p, sizeof p, "%s/%s", strcmp(dir, "/") ? dir : "", e->d_name);
		if (lstat(p, &s) || s.st_dev != want->st_dev || s.st_ino != want->st_ino) continue;
		strcpy(name, e->d_name);
		closedir(d);
		return 0;
	}
	closedir(d);
	errno = ENOENT;
	return -1;
}

#define SAME(a, b) ((a).st_dev == (b).st_dev && (a).st_ino == (b).st_ino)

char *getcwd(char *buf, size_t size)
{
	static char cache[PATH_MAX];
	static struct stat cst;
	char path[PATH_MAX], up[PATH_MAX], name[MAXNAMLEN + 1];
	char *p = path + sizeof path - 1;
	struct stat cur, par, root, dot;
	int n, top;

	if (stat(".", &dot) || stat("/", &root)) return 0;
	if (!*cache || !SAME(dot, cst)) {
		*p = 0;
		cur = dot;
		strcpy(up, ".");
		while (!SAME(cur, root)) {
			if (strlen(up) + 4 > sizeof up) goto range;
			strcat(up, "/..");
			if (stat(up, &par)) return 0;
			top = SAME(par, cur) || SAME(par, root);
			if (lookup(top ? "/" : up, &cur, name)) return 0;
			n = strlen(name);
			if (p - path < n + 1) goto range;
			p -= n;
			memcpy(p, name, n);
			*--p = '/';
			if (top) break;
			cur = par;
		}
		strcpy(cache, *p ? p : "/");
		cst = dot;
	}
	if (!buf && !(buf = malloc(size = size ? size : strlen(cache) + 1))) return 0;
	if (strlen(cache) >= size) goto range;
	return strcpy(buf, cache);
range:
	errno = ERANGE;
	return 0;
}

char *getwd(char *buf) { return getcwd(buf, PATH_MAX); }

