/* netOS has no users: everyone is root, at home in $HOME */
#include <grp.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>

static struct passwd pw;
static struct group gr;
static char *mem[] = { "root", 0 };
static int ent;

struct passwd *getpwuid(uid_t u)
{
	const char *h = getenv("HOME");

	pw.pw_name = "root";
	pw.pw_passwd = "";
	pw.pw_uid = u;
	pw.pw_gid = 0;
	pw.pw_gecos = "netOS";
	pw.pw_dir = h ? (char *) h : "/";
	pw.pw_shell = "/bin/sh";
	return &pw;
}

struct passwd *getpwnam(const char *n) { return strcmp(n, "root") ? 0 : getpwuid(0); }
struct passwd *getpwent(void) { return ent++ ? 0 : getpwuid(0); }
void setpwent(void) { ent = 0; }
void endpwent(void) { ent = 0; }
char *getlogin(void) { return "root"; }

struct group *getgrgid(gid_t g)
{
	gr.gr_name = "root";
	gr.gr_passwd = "";
	gr.gr_gid = g;
	gr.gr_mem = mem;
	return &gr;
}

struct group *getgrnam(const char *n) { return strcmp(n, "root") ? 0 : getgrgid(0); }
struct group *getgrent(void) { return ent++ ? 0 : getgrgid(0); }
void setgrent(void) { ent = 0; }
void endgrent(void) { ent = 0; }
