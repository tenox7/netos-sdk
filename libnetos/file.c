/* files: netOS flag values and its 4.3BSD struct stat, translated */
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>
#include <sys/stat.h>
#include <sys/time.h>

extern int netos_open(const char *, int, int);
extern int netos_stat(const char *, void *);
extern int netos_lstat(const char *, void *);
extern int netos_fstat(int, void *);
extern int netos_fcntl(int, int, int);
extern int netos_ioctl(int, unsigned long, void *);
extern int netos_mkdir(const char *, int);
extern int netos_utimes(const char *, const struct timeval *);

/* netOS open takes FREAD|FWRITE (1, 2, 3) for the access mode, and its own
   append and non-blocking bits */
#define KNDELAY	0x04
#define KAPPEND	0x10

static int kflags(int f)
{
	return ((f & O_ACCMODE) + 1) | (f & (O_CREAT | O_TRUNC | O_EXCL)) |
	       (f & O_APPEND ? KAPPEND : 0) | (f & O_NONBLOCK ? KNDELAY : 0);
}

static int uflags(int k)
{
	static const int acc[] = { O_RDWR, O_RDONLY, O_WRONLY, O_RDWR };

	return acc[k & 3] | (k & KAPPEND ? O_APPEND : 0) | (k & KNDELAY ? O_NONBLOCK : 0);
}

int open(const char *path, int flags, ...)
{
	va_list ap;
	int mode, fd, e = errno, k = kflags(flags);

	va_start(ap, flags);
	mode = va_arg(ap, int);
	va_end(ap);
	if (!(flags & O_CREAT)) mode = 0;
	fd = netos_open(path, k, mode);
	/* netOS's NFS client refuses to open for writing what its own permission
	   check dislikes, though the server takes the writes; everyone is root */
	if (fd < 0 && errno == EACCES && (k & 2) && (fd = netos_open(path, (k & ~3) | 1, mode)) >= 0)
		errno = e;
	return fd;
}

int creat(const char *path, mode_t mode)
{
	return open(path, O_WRONLY | O_CREAT | O_TRUNC, mode);
}

/* netOS keeps F_GETFL and F_SETFL for devices; the rest come from the flags
   ioctl, which knows pipes and sockets, and FIONBIO */
int fcntl(int fd, int cmd, ...)
{
	va_list ap;
	int a, r, e = errno, fl = 0, on;

	va_start(ap, cmd);
	a = va_arg(ap, int);
	va_end(ap);
	if (cmd == F_SETFL) {
		if (netos_fcntl(fd, cmd, kflags(a) & (KAPPEND | KNDELAY)) != -1) return 0;
		on = (a & O_NONBLOCK) != 0;
		return netos_ioctl(fd, 0x6602, &on);
	}
	if (cmd != F_GETFL) return netos_fcntl(fd, cmd, a);
	if ((r = netos_fcntl(fd, cmd, 0)) != -1) return uflags(r);
	netos_ioctl(fd, 0x6608, &fl);
	errno = e;
	return fl ? uflags(fl) : O_RDWR;
}

/* netOS returns the 4.3BSD struct stat */
struct kstat {
	short		dev, pad0;
	unsigned long	ino;
	unsigned short	mode;
	short		nlink;
	unsigned short	uid, gid;
	short		rdev, pad1;
	long		size, atime, sp1, mtime, sp2, ctime, sp3, blksize, blocks, sp4[2];
};

static int kst(int r, struct kstat *k, struct stat *s)
{
	if (r < 0) return r;
	memset(s, 0, sizeof *s);
	s->st_dev = k->dev;
	s->st_ino = k->ino;
	s->st_mode = k->mode;
	s->st_nlink = k->nlink;
	s->st_uid = k->uid;
	s->st_gid = k->gid;
	s->st_rdev = k->rdev;
	s->st_size = k->size;
	s->st_atime = k->atime;
	s->st_mtime = k->mtime;
	s->st_ctime = k->ctime;
	s->st_blksize = k->blksize;
	s->st_blocks = k->blocks;
	return 0;
}

int stat(const char *p, struct stat *s)
{
	struct kstat k;
	return kst(netos_stat(p, &k), &k, s);
}

int lstat(const char *p, struct stat *s)
{
	struct kstat k;
	return kst(netos_lstat(p, &k), &k, s);
}

/* sockets, pipes and ptys have no inode; make one up */
int fstat(int fd, struct stat *s)
{
	struct kstat k;
	int e = errno, fl = 0;

	if (netos_fstat(fd, &k) == 0) return kst(0, &k, s);
	if (errno != EPERM) return -1;
	errno = e;
	memset(s, 0, sizeof *s);
	netos_ioctl(fd, 0x6608, &fl);
	s->st_mode = isatty(fd) ? S_IFCHR | 0620 : (fl & 3) == 3 ? S_IFSOCK | 0777 : S_IFIFO | 0600;
	s->st_nlink = 1;
	s->st_blksize = 1024;
	errno = e;
	return 0;
}

int access(const char *p, int m)
{
	struct stat s;

	if (stat(p, &s)) return -1;
	if ((m & R_OK && !(s.st_mode & 0444)) || (m & W_OK && !(s.st_mode & 0222)) ||
	    (m & X_OK && !(s.st_mode & 0111))) {
		errno = EACCES;
		return -1;
	}
	return 0;
}

int truncate(const char *p, off_t n)
{
	int fd = open(p, O_WRONLY), r;

	if (fd < 0) return -1;
	r = ftruncate(fd, n);
	close(fd);
	return r;
}

int utime(const char *p, const struct utimbuf *u)
{
	struct timeval t[2];

	if (!u) return utimes(p, 0);
	t[0].tv_sec = u->actime;
	t[1].tv_sec = u->modtime;
	t[0].tv_usec = t[1].tv_usec = 0;
	return utimes(p, t);
}

int lchown(const char *p, uid_t u, gid_t g) { return chown(p, u, g); }
void sync(void) {}
int fsync(int fd) { return 0; }

/* netOS fails mkdir("/") with ENOENT */
int mkdir(const char *p, mode_t m)
{
	struct stat s;
	int e;

	if (!netos_mkdir(p, m)) return 0;
	e = errno;
	errno = stat(p, &s) ? e : EEXIST;
	return -1;
}

/* netOS takes whole seconds only, like its own mv passes, and no NULL */
int utimes(const char *p, const struct timeval *t)
{
	struct timeval tv[2];

	if (t) {
		tv[0] = t[0];
		tv[1] = t[1];
	} else {
		gettimeofday(tv, 0);
		tv[1] = tv[0];
	}
	tv[0].tv_usec = tv[1].tv_usec = 0;
	return netos_utimes(p, tv);
}

/* netOS readlink (7/0x45) hangs the whole Station */
int readlink(const char *p, char *b, size_t n)
{
	errno = EINVAL;
	return -1;
}
