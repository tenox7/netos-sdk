/* Directories through the kernel's own opendir/readdir, which hand back
   SunOS-style entries: d_off, d_fileno, d_reclen, d_namlen, d_name. */
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

struct kdirent {
	long		d_off;
	unsigned long	d_fileno;
	unsigned short	d_reclen, d_namlen;
	char		d_name[1];
};

extern void *netos_opendir(const char *);
extern struct kdirent *netos_readdir(void *);
extern int netos_closedir(void *);
extern int __netos_seterr(void);

DIR *opendir(const char *path)
{
	DIR *d = calloc(1, sizeof *d);

	if (!d) return 0;
	if ((d->dd_path = strdup(path)) && (d->dd_k = netos_opendir(path))) return d;
	if (d->dd_path) __netos_seterr();
	free(d->dd_path);
	free(d);
	return 0;
}

struct dirent *readdir(DIR *d)
{
	struct kdirent *k;
	int n;

	if (!d->dd_k || !(k = netos_readdir(d->dd_k))) return 0;
	n = k->d_namlen > MAXNAMLEN ? MAXNAMLEN : k->d_namlen;
	d->dd_ent.d_ino = k->d_fileno;
	d->dd_ent.d_reclen = sizeof d->dd_ent;
	d->dd_ent.d_namlen = n;
	memcpy(d->dd_ent.d_name, k->d_name, n);
	d->dd_ent.d_name[n] = 0;
	d->dd_loc++;
	return &d->dd_ent;
}

int closedir(DIR *d)
{
	int r = d->dd_k ? netos_closedir(d->dd_k) : 0;

	free(d->dd_path);
	free(d);
	return r;
}

void rewinddir(DIR *d)
{
	if (d->dd_k) netos_closedir(d->dd_k);
	d->dd_k = netos_opendir(d->dd_path);
	d->dd_loc = 0;
}

long telldir(DIR *d) { return d->dd_loc; }

void seekdir(DIR *d, long loc)
{
	rewinddir(d);
	while (d->dd_loc < loc && readdir(d))
		;
}

int dirfd(DIR *d) { return d->dd_k ? *(int *) d->dd_k : -1; }

int alphasort(const struct dirent **a, const struct dirent **b)
{
	return strcmp((*a)->d_name, (*b)->d_name);
}

int scandir(const char *path, struct dirent ***list, int (*sel)(const struct dirent *),
	    int (*cmp)(const struct dirent **, const struct dirent **))
{
	struct dirent *e, **v = 0, **t;
	int n = 0, max = 0;
	DIR *d = opendir(path);

	if (!d) return -1;
	while ((e = readdir(d))) {
		if (sel && !sel(e)) continue;
		if (n == max) {
			if (!(t = realloc(v, (max = max * 2 + 16) * sizeof *v))) goto fail;
			v = t;
		}
		if (!(v[n] = malloc(sizeof *e))) goto fail;
		*v[n++] = *e;
	}
	closedir(d);
	if (cmp && n) qsort(v, n, sizeof *v, (int (*)(const void *, const void *)) cmp);
	*list = v;
	return n;
fail:
	while (n--) free(v[n]);
	free(v);
	closedir(d);
	errno = ENOMEM;
	return -1;
}
