#ifndef _TIME_H
#define _TIME_H
typedef long time_t;
struct tm {
	int tm_sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year;
	int tm_wday, tm_yday, tm_isdst;
	long tm_gmtoff;
	char *tm_zone;
};
time_t time(time_t *);
struct tm *localtime(const time_t *);
struct tm *gmtime(const time_t *);
char *ctime(const time_t *);
char *asctime(const struct tm *);
#endif
