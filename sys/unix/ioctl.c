/*	SCCS Id: @(#)ioctl.c	3.1	90/22/02
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* This cannot be part of hack.tty.c (as it was earlier) since on some
   systems (e.g. MUNIX) the include files <termio.h> and <sgtty.h>
   define the same constants, and the C preprocessor complains. */

#include "hack.h"

#if defined(BSD_JOB_CONTROL) || defined(_BULL_SOURCE)
# ifdef HPUX
#include <bsdtty.h>
# else
#  if defined(AIX_31) && !defined(_ALL_SOURCE)
#   define _ALL_SOURCE	/* causes struct winsize to be present */
#  endif
#  if defined(_BULL_SOURCE)
#   include <termios.h>
struct termios termio;
#   undef TIMEOUT		/* defined in you.h and sys/tty.h */
#   include <sys/tty.h>		/* define winsize */
#   include <sys/ttold.h>	/* define struct ltchars */
#   include <sys/bsdioctl.h>	/* define TIOGWINSZ */
#  else
#   include <sgtty.h>
#  endif
# endif
struct ltchars ltchars;
struct ltchars ltchars0 = { -1, -1, -1, -1, -1, -1 }; /* turn all off */
#else

# ifdef POSIX_TYPES
#include <termios.h>
struct termios termio;
#  ifdef BSD
#include <sys/ioctl.h>
#  endif
# else
#include <termio.h>	/* also includes part of <sgtty.h> */
#  if defined(TCSETS) && !defined(AIX_31)
struct termios termio;
#  else
struct termio termio;
#  endif
# endif
# ifdef AMIX
#include <sys/ioctl.h>
# endif /* AMIX */
#endif

#ifdef SUSPEND	/* BSD isn't alone anymore... */
#include	<signal.h>
#endif

#if defined(TIOCGWINSZ) && (defined(BSD) || defined(ULTRIX) || defined(AIX_31) || defined(_BULL_SOURCE) || defined(SVR4))
#define USE_WIN_IOCTL
#include "termcap.h"	/* for LI and CO */
#endif

#ifdef AUX
void *
catch_stp ( )
{
    signal ( SIGTSTP , SIG_DFL ) ;
    dosuspend ( ) ;
}
#endif /* AUX */

#ifdef USE_WIN_IOCTL
void
getwindowsz()
{
    /*
     * ttysize is found on Suns and BSD
     * winsize is found on Suns, BSD, and Ultrix
     */
    struct winsize ttsz;

    if (ioctl(fileno(stdin), (int)TIOCGWINSZ, (char *)&ttsz) != -1) {
	/*
	 * Use the kernel's values for lines and columns if it has
	 * any idea.
	 */
	if (ttsz.ws_row)
	    LI = ttsz.ws_row;
	if (ttsz.ws_col)
	    CO = ttsz.ws_col;
    }
}
#endif

void
getioctls()
{
#ifdef BSD_JOB_CONTROL
	(void) ioctl(fileno(stdin), (int) TIOCGLTC, (char *) &ltchars);
	(void) ioctl(fileno(stdin), (int) TIOCSLTC, (char *) &ltchars0);
#else
# ifdef POSIX_TYPES
	(void) tcgetattr(fileno(stdin), &termio);
# else
#  if defined(TCSETS) && !defined(AIX_31)
	(void) ioctl(fileno(stdin), (int) TCGETS, &termio);
#  else
	(void) ioctl(fileno(stdin), (int) TCGETA, &termio);
#  endif
# endif
#endif
#ifdef USE_WIN_IOCTL
	getwindowsz();
#endif
#ifdef AUX
	( void ) signal ( SIGTSTP , catch_stp ) ;
#endif
}

void
setioctls()
{
#ifdef BSD_JOB_CONTROL
	(void) ioctl(fileno(stdin), (int) TIOCSLTC, (char *) &ltchars);
#else
# ifdef POSIX_TYPES
	(void) tcsetattr(fileno(stdin), TCSADRAIN, &termio);
# else
#  if defined(TCSETS) && !defined(AIX_31)
	(void) ioctl(fileno(stdin), (int) TCSETSW, &termio);
#  else
	(void) ioctl(fileno(stdin), (int) TCSETAW, &termio);
#  endif
# endif
#endif
}

#ifdef SUSPEND		/* Does not imply BSD */
int
dosuspend()
{
#ifdef SIGTSTP
	if(signal(SIGTSTP, SIG_IGN) == SIG_DFL) {
		suspend_nhwindows(NULL);
		(void) signal(SIGTSTP, SIG_DFL);
#ifdef AUX
		( void ) kill ( 0 , SIGSTOP ) ;
#else
		(void) kill(0, SIGTSTP);
#endif
		resume_nhwindows();
	} else {
		pline("I don't think your shell has job control.");
	}
#else
	pline("Sorry, it seems we have no SIGTSTP here.  Try ! or S.");
#endif
	return(0);
}
#endif /* SUSPEND */