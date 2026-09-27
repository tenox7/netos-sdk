#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/times.h>

int nanosleep(const struct timespec *t, struct timespec *rem)
{
	struct timeval tv;

	tv.tv_sec = t->tv_sec;
	tv.tv_usec = t->tv_nsec / 1000;
	if (rem) rem->tv_sec = rem->tv_nsec = 0;
	return select(0, 0, 0, 0, &tv) < 0 ? -1 : 0;
}

int usleep(useconds_t us)
{
	struct timeval tv;

	tv.tv_sec = us / 1000000;
	tv.tv_usec = us % 1000000;
	return select(0, 0, 0, 0, &tv) < 0 ? -1 : 0;
}

unsigned sleep(unsigned s)
{
	struct timeval tv;

	tv.tv_sec = s;
	tv.tv_usec = 0;
	select(0, 0, 0, 0, &tv);
	return 0;
}

clock_t times(struct tms *t)
{
	static struct timeval t0;
	struct timeval tv;
	clock_t c;

	gettimeofday(&tv, 0);
	if (!t0.tv_sec) t0 = tv;
	c = (tv.tv_sec - t0.tv_sec) * CLOCKS_PER_SEC + (tv.tv_usec - t0.tv_usec) / (1000000 / CLOCKS_PER_SEC);
	if (t) {
		t->tms_utime = c;
		t->tms_stime = t->tms_cutime = t->tms_cstime = 0;
	}
	return c;
}

/* the clock is set from the network at boot */
int settimeofday(const struct timeval *tv, const struct timezone *tz)
{
	errno = EPERM;
	return -1;
}

int stime(const time_t *t)
{
	errno = EPERM;
	return -1;
}

/* no interval timers: nothing would deliver SIGALRM to newlib's handlers */
int setitimer(int which, const struct itimerval *v, struct itimerval *old)
{
	errno = ENOSYS;
	return -1;
}

int getitimer(int which, struct itimerval *v)
{
	errno = ENOSYS;
	return -1;
}
