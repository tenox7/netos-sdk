/* newlib's, plus the sigaction flags it leaves out */
#include_next <signal.h>
#ifndef _NETOS_SIGNAL_H
#define _NETOS_SIGNAL_H
typedef void (*sighandler_t)(int);
#define _NSIG	NSIG
#endif
#ifndef SA_RESTART
#define SA_ONSTACK	0x0010
#define SA_RESTART	0x0020
#define SA_NODEFER	0x0040
#define SA_RESETHAND	0x0080
#endif
