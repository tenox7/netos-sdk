#ifndef _SYS_DIRENT_H
#define _SYS_DIRENT_H
#define MAXNAMLEN	255

struct dirent {
	long		d_ino;
	unsigned short	d_reclen;
	unsigned short	d_namlen;
	char		d_name[MAXNAMLEN + 1];
};
#define d_fileno	d_ino

typedef struct {
	void		*dd_k;		/* the kernel's DIR */
	char		*dd_path;
	long		dd_loc;
	struct dirent	dd_ent;
} DIR;

DIR		*opendir(const char *);
struct dirent	*readdir(DIR *);
int		closedir(DIR *);
void		rewinddir(DIR *);
long		telldir(DIR *);
void		seekdir(DIR *, long);
int		dirfd(DIR *);
int		alphasort(const struct dirent **, const struct dirent **);
int		scandir(const char *, struct dirent ***, int (*)(const struct dirent *),
			int (*)(const struct dirent **, const struct dirent **));
#endif
