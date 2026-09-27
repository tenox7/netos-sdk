/*
 * This code contains changes by
 *      Gunnar Ritter, Freiburg i. Br., Germany, 2002. All rights reserved.
 *
 * Conditions 1, 2, and 4 and the no-warranty notice below apply
 * to these changes.
 *
 *
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef	lint
#ifdef	DOSCCS
static char *sccsid = "@(#)termcap.c	1.7 (gritter) 11/23/04";
#endif
#endif

/* from termcap.c	5.1 (Berkeley) 6/5/85 */

#if 0	/* GR */
#define	TCBUFSIZE		1024
#else
#include "termcap.h"
#endif
#define	E_TERMCAP	"/etc/termcap"
#define MAXHOP		32	/* max number of tc= indirections */

/*
 * netOS has no /etc/termcap, so the terminals it meets are built in: its
 * Console Window emulates a VT320, and telnet clients speak VT220 or xterm.
 */
static const char *const builtin[] = {
	"vt220|vt200|vt300|vt320|vt420|vt520|dec vt220:"
	"am:mi:ms:xn:xo:bs:pt:co#80:li#24:it#8:"
	"AL=\\E[%dL:DL=\\E[%dM:DC=\\E[%dP:IC=\\E[%d@:al=\\E[L:dl=\\E[M:dc=\\E[P:ic=\\E[@:"
	"im=\\E[4h:ei=\\E[4l:cd=\\E[J:ce=\\E[K:cl=\\E[H\\E[J:cm=\\E[%i%d;%dH:"
	"cs=\\E[%i%d;%dr:cr=^M:do=^J:ho=\\E[H:le=^H:nd=\\E[C:up=\\E[A:sf=^J:sr=\\EM:"
	"ta=^I:sc=\\E7:rc=\\E8:bl=^G:vi=\\E[?25l:ve=\\E[?25h:"
	"md=\\E[1m:mb=\\E[5m:me=\\E[m:mr=\\E[7m:so=\\E[7m:se=\\E[m:us=\\E[4m:ue=\\E[m:"
	"as=\\E(0:ae=\\E(B:ac=``aaffggjjkkllmmnnooppqqrrssttuuvvwwxxyyzz{{||}}~~:"
	"ks=\\E[?1h\\E=:ke=\\E[?1l\\E>:kb=^H:ku=\\EOA:kd=\\EOB:kr=\\EOC:kl=\\EOD:"
	"kh=\\E[1~:@7=\\E[4~:kI=\\E[2~:kD=\\E[3~:kP=\\E[5~:kN=\\E[6~:"
	"k1=\\EOP:k2=\\EOQ:k3=\\EOR:k4=\\EOS:k5=\\E[17~:k6=\\E[18~:"
	"k7=\\E[19~:k8=\\E[20~:k9=\\E[21~:k;=\\E[23~:",
	"vt100|vt102|xterm|xterm-color|xterm-256color|rxvt|screen|screen-256color|"
	"tmux|tmux-256color|linux|putty|ansi:tc=vt220:",
	"dumb|unknown|network|dialup:am:co#80:bl=^G:cr=^M:do=^J:sf=^J:",
	0
};

#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

/*
 * termcap - routines for dealing with the terminal capability data base
 *
 * BUG:		Should use a "last" pointer in tbuf, so that searching
 *		for capabilities alphabetically would not be a n**2/2
 *		process when large numbers of capabilities are given.
 * Note:	If we add a last pointer now we will screw up the
 *		tc capability. We really should compile termcap.
 *
 * Essentially all the work here is scanning and decoding escapes
 * in string capabilities.  We don't use stdio because the editor
 * doesn't, and because living w/o it is not hard.
 */

static	char *tbuf;
static	int hopcount;	/* detect infinite loops in termcap, init 0 */

static int tnamatch(const char *);
static int tnchktc(void);
static char *tskip(register const char *);
static char *tdecode(register char *, char **);

/*
 * Tnamatch deals with name matching.  The first field of the termcap
 * entry is a sequence of names separated by |'s, so we compare
 * against each such name.  The normal : terminator after the last
 * name (before the first field) stops us.
 */
static int
tnamatch(const char *np)
{
	register const char *Np;
	register char *Bp;

	Bp = tbuf;
	if (*Bp == '#')
		return(0);
	for (;;) {
		for (Np = np; *Np && *Bp == *Np; Bp++, Np++)
			continue;
		if (*Np == 0 && (*Bp == '|' || *Bp == ':' || *Bp == 0))
			return (1);
		while (*Bp && *Bp != ':' && *Bp != '|')
			Bp++;
		if (*Bp == 0 || *Bp == ':')
			return (0);
		Bp++;
	}
}

/*
 * tnchktc: check the last entry, see if it's tc=xxx. If so,
 * recursively find xxx and append that entry (minus the names)
 * to take the place of the tc=xxx entry. This allows termcap
 * entries to say "like an HP2621 but doesn't turn on the labels".
 * Note that this works because of the left to right scan.
 */
static int
tnchktc(void)
{
	register char *p, *q;
	char tcname[16];	/* name of similar terminal */
	char tcbuf[TCBUFSIZE];
	char rmbuf[TCBUFSIZE];
	char *holdtbuf = tbuf, *holdtc;
	int l;

	p = tbuf;
	while (*p) {
		holdtc = p = tskip(p);
		if (!*p)
			break;
		if (*p++ != 't' || *p == 0 || *p++ != 'c')
			continue;
		if (*p++ != '=') {
		bad:	write(2, "Bad termcap entry\n", 18);
			return (0);
		}
		for (q = tcname; *p && *p != ':'; p++) {
			if (q >= &tcname[sizeof tcname - 1])
				goto bad;
			*q++ = *p;
		}
		*q = '\0';
		if (++hopcount > MAXHOP) {
			write(2, "Infinite tc= loop\n", 18);
			return (0);
		}
		if (tgetent(tcbuf, tcname) != 1) {
			hopcount = 0;		/* unwind recursion */
			return(0);
		}
		hopcount--;
		tbuf = holdtbuf;
		strcpy(rmbuf, &p[1]);
		for (q=tcbuf; *q != ':'; q++)
			;
		l = holdtc - holdtbuf + strlen(rmbuf) + strlen(q);
		if (l > TCBUFSIZE) {
			write(2, "Termcap entry too long\n", 23);
			break;
		}
		q++;
		for (p = holdtc; *q; q++)
			*p++ = *q;
		strcpy(p, rmbuf);
		p = holdtc;
	}
	return(1);
}

static int
tbuiltin(char *bp, const char *name)
{
	const char *const *e;

	for (e = builtin; *e; e++) {
		tbuf = (char *) *e;
		if (tnamatch(name)) {
			tbuf = bp;
			strcpy(bp, *e);
			return(tnchktc());
		}
	}
	tbuf = bp;
	return (0);
}

/*
 * Get an entry for terminal name in buffer bp,
 * from the termcap file.  Parse is very rudimentary;
 * we just notice escaped newlines.
 */
int
tgetent(char *bp, const char *name)
{
	register char *cp;
	register int c;
	register int i = 0, cnt = 0;
	char ibuf[TCBUFSIZE];
	int tf;

	if (!name || !*name)
		name = "vt220";
	tbuf = bp;
	tf = -1;
#ifndef V6
	cp = getenv("TERMCAP");
	/*
	 * TERMCAP can have one of two things in it. It can be the
	 * name of a file to use instead of /etc/termcap. In this
	 * case it better start with a "/". Or it can be an entry to
	 * use so we don't have to read the file. In this case it
	 * has to already have the newlines crunched out.
	 */
	if (cp && *cp) {
		if (*cp == '/') {
			tf = open(cp, 0);
		} else {
			tbuf = cp;
			c = tnamatch(name);
			tbuf = bp;
			if (c) {
				strcpy(bp,cp);
				return(tnchktc());
			}
		}
	}
	if (tf < 0)
		tf = open(E_TERMCAP, 0);
#else
	tf = open(E_TERMCAP, 0);
#endif
	if (tf < 0)
		return (tbuiltin(bp, name));
	for (;;) {
		cp = bp;
		for (;;) {
			if (i == cnt) {
				cnt = read(tf, ibuf, TCBUFSIZE);
				if (cnt <= 0) {
					close(tf);
					return (tbuiltin(bp, name));
				}
				i = 0;
			}
			c = ibuf[i++];
			if (c == '\n') {
				if (cp > bp && cp[-1] == '\\'){
					cp--;
					continue;
				}
				break;
			}
			if (cp >= bp+TCBUFSIZE) {
				write(2,"Termcap entry too long\n", 23);
				break;
			} else
				*cp++ = c;
		}
		*cp = 0;

		/*
		 * The real work for the match.
		 */
		if (tnamatch(name)) {
			close(tf);
			return(tnchktc());
		}
	}
}

/*
 * Skip to the next field.  Notice that this is very dumb, not
 * knowing about \: escapes or any such.  If necessary, :'s can be put
 * into the termcap file in octal.
 */
static char *
tskip(register const char *bp)
{

	while (*bp && *bp != ':')
		bp++;
	if (*bp == ':')
		bp++;
	return (char *)bp;
}

/*
 * Return the (numeric) option id.
 * Numeric options look like
 *	li#80
 * i.e. the option string is separated from the numeric value by
 * a # character.  If the option is not found we return -1.
 * Note that we handle octal numbers beginning with 0.
 */
int
tgetnum(char *id)
{
	register int i, base;
	register char *bp = tbuf;

	for (;;) {
		bp = tskip(bp);
		if (*bp == 0)
			return (-1);
		if (*bp++ != id[0] || *bp == 0 || *bp++ != id[1])
			continue;
		if (*bp == '@')
			return(-1);
		if (*bp != '#')
			continue;
		bp++;
		base = 10;
		if (*bp == '0')
			base = 8;
		i = 0;
		while (isdigit((*bp & 0377)))
			i *= base, i += *bp++ - '0';
		return (i);
	}
}

/*
 * Handle a flag option.
 * Flag options are given "naked", i.e. followed by a : or the end
 * of the buffer.  Return 1 if we find the option, or 0 if it is
 * not given.
 */
int
tgetflag(char *id)
{
	register char *bp = tbuf;

	for (;;) {
		bp = tskip(bp);
		if (!*bp)
			return (0);
		if (*bp++ == id[0] && *bp != 0 && *bp++ == id[1]) {
			if (!*bp || *bp == ':')
				return (1);
			else if (*bp == '@')
				return(0);
		}
	}
}

/*
 * Get a string valued option.
 * These are given as
 *	cl=^Z
 * Much decoding is done on the strings, and the strings are
 * placed in area, which is a ref parameter which is updated.
 * No checking on area overflow.
 */
char *
tgetstr(char *id, char **area)
{
	register char *bp = tbuf;

	for (;;) {
		bp = tskip(bp);
		if (!*bp)
			return (0);
		if (*bp++ != id[0] || *bp == 0 || *bp++ != id[1])
			continue;
		if (*bp == '@')
			return(0);
		if (*bp != '=')
			continue;
		bp++;
		return (tdecode(bp, area));
	}
}

/*
 * Tdecode does the grung work to decode the
 * string capability escapes.
 */
static char *
tdecode(register char *str, char **area)
{
	register char *cp;
	register int c;
	register char *dp;
	int i;

	cp = *area;
	while ((c = *str++) && c != ':') {
		switch (c) {

		case '^':
			c = *str++ & 037;
			break;

		case '\\':
			dp = "E\033^^\\\\::n\nr\rt\tb\bf\f";
			c = *str++;
nextc:
			if (*dp++ == c) {
				c = *dp++;
				break;
			}
			dp++;
			if (*dp)
				goto nextc;
			if (isdigit(c)) {
				c -= '0', i = 2;
				do
					c <<= 3, c |= *str++ - '0';
				while (--i && isdigit(*str & 0377));
			}
			break;
		}
		*cp++ = c;
	}
	*cp++ = 0;
	str = *area;
	*area = cp;
	return (str);
}

/*
*/
static const char sccssl[] = "@(#)libterm.sl	1.7 (gritter) 11/23/04";
