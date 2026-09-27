#ifndef _TERMCAP_H
#define _TERMCAP_H
/* the 2.11BSD termcap library, with the common terminals built in */
#define	TCBUFSIZE	2048

extern char PC, *UP, *BC;
extern short ospeed;

int	tgetent(char *, const char *);
int	tgetflag(char *);
int	tgetnum(char *);
char	*tgetstr(char *, char **);
char	*tgoto(char *, int, int);
int	tputs(const char *, int, int (*)(int));
#endif
