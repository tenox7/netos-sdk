/* malloc on the kernel heap (netOS 10/0x2a malloc, 10/0x1c free), which
   frees a task's blocks when it exits.  Each block keeps its size and the
   kernel's pointer just below the aligned payload; the heap is the kernel's
   own, so free() refuses anything that is not a live block.  A vforked
   child runs in our memory, but the kernel frees what the child allocates
   when it execs or exits, so the child borrows from a pool of ours. */
#include <errno.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <reent.h>

#define ALIGN_MAX 4096

extern void *netos_malloc(size_t);
extern void netos_free(void *);
extern int __netos_vforked;

static double pool[2048];
static size_t used;

static void *get(size_t n, size_t align)
{
	char *r, *p;

	if (align > ALIGN_MAX || n > 0x7fffffff - align - 8) goto fail;
	if (__netos_vforked) {
		if (used + n + align + 8 > sizeof pool) goto fail;
		r = (char *) pool + used;
		used += n + align + 8;
	} else if (!(r = netos_malloc(n + align + 8)))
		goto fail;
	p = (char *) (((unsigned long) r + 8 + align - 1) & ~(align - 1));
	((size_t *) p)[-2] = n;
	((char **) p)[-1] = __netos_vforked ? 0 : r;
	return p;
fail:
	errno = ENOMEM;
	return 0;
}

void *malloc(size_t n) { return get(n ? n : 1, 8); }
void *memalign(size_t a, size_t n) { return get(n ? n : 1, a < 8 ? 8 : a); }
void *valloc(size_t n) { return memalign(4096, n); }
size_t malloc_usable_size(void *p) { return p ? ((size_t *) p)[-2] : 0; }

void free(void *p)
{
	char *r;

	if (!p || !(r = ((char **) p)[-1])) return;
	if ((unsigned long) ((char *) p - r) - 8 > ALIGN_MAX) {
		write(2, "free: bad pointer\n", 18);
		return;
	}
	((char **) p)[-1] = 0;
	netos_free(r);
}

void *calloc(size_t a, size_t b)
{
	void *p;

	if (b && a > 0x7fffffff / b) {
		errno = ENOMEM;
		return 0;
	}
	if ((p = malloc(a * b))) memset(p, 0, a * b);
	return p;
}

void *realloc(void *p, size_t n)
{
	size_t old;
	void *q;

	if (!p) return malloc(n);
	if (!n) {
		free(p);
		return 0;
	}
	if ((old = ((size_t *) p)[-2]) >= n) return p;
	if (!(q = malloc(n))) return 0;
	memcpy(q, p, old);
	free(p);
	return q;
}

void *_malloc_r(struct _reent *r, size_t n) { return malloc(n); }
void *_calloc_r(struct _reent *r, size_t a, size_t b) { return calloc(a, b); }
void *_realloc_r(struct _reent *r, void *p, size_t n) { return realloc(p, n); }
void *_memalign_r(struct _reent *r, size_t a, size_t n) { return memalign(a, n); }
void _free_r(struct _reent *r, void *p) { free(p); }
size_t _malloc_usable_size_r(struct _reent *r, void *p) { return malloc_usable_size(p); }
void __malloc_lock(struct _reent *r) {}
void __malloc_unlock(struct _reent *r) {}
