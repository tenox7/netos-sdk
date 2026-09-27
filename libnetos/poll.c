#include <errno.h>
#include <poll.h>
#include <sys/select.h>
#include <sys/time.h>

int poll(struct pollfd *p, nfds_t n, int ms)
{
	struct timeval tv;
	fd_set r, w, e;
	int i, max = -1, k;

	FD_ZERO(&r);
	FD_ZERO(&w);
	FD_ZERO(&e);
	for (i = 0; i < (int) n; i++) {
		p[i].revents = 0;
		if (p[i].fd < 0) continue;
		if (p[i].events & (POLLIN | POLLRDNORM)) FD_SET(p[i].fd, &r);
		if (p[i].events & POLLOUT) FD_SET(p[i].fd, &w);
		FD_SET(p[i].fd, &e);
		if (p[i].fd > max) max = p[i].fd;
	}
	tv.tv_sec = ms / 1000;
	tv.tv_usec = ms % 1000 * 1000;
	if ((k = select(max + 1, &r, &w, &e, ms < 0 ? 0 : &tv)) <= 0) return k;
	for (k = i = 0; i < (int) n; i++) {
		if (p[i].fd < 0) continue;
		if (FD_ISSET(p[i].fd, &r)) p[i].revents |= p[i].events & (POLLIN | POLLRDNORM);
		if (FD_ISSET(p[i].fd, &w)) p[i].revents |= POLLOUT;
		if (FD_ISSET(p[i].fd, &e)) p[i].revents |= POLLERR;
		if (p[i].revents) k++;
	}
	return k;
}
