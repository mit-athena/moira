/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/display.c,v $
 *	$Author: mar $
 *	$Locker:  $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/display.c,v 1.9 1991-07-01 14:29:49 mar Exp $
 *
 *  (c) Copyright 1988 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  <mit-copyright.h>.
 */

#ifndef lint
static char *rcsid_display_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/display.c,v 1.9 1991-07-01 14:29:49 mar Exp $";
#endif	lint

#include <mit-copyright.h>
#include <stdio.h>
#include <curses.h>
#include <sys/time.h>
#include "userreg.h"

#define DESC_WIDTH 18
#define HEADER "*** Project Athena User Registration ***"
#if defined(vax) || defined(mips)
#define HELP   " Press the key above RETURN to delete a character.  Press Ctrl-C to start over."
#endif
#ifndef HELP
#define HELP   " Press backspace to delete a character.  Press Ctrl-C to start over."
#endif
#define BORDER_CHAR '-'
#define MIN_COLS 80
#define MIN_LINES 24

WINDOW * displayw, *queryw;
WINDOW * dataw, *helpw;
WINDOW * fnamew, *midw, *lnamew, *idw, *loginw;

/* Set up the windows and subwindows on the display */
setup_display () {
  FILE * freopen ();

  initscr ();			/* Start up curses */

  if (COLS < MIN_COLS || LINES < MIN_LINES) {
    fprintf (stderr, "Screen must be at least %d x %d\n", MIN_LINES, MIN_COLS);
    exit (1);
  }

  noecho ();			/* And the tty input */
  raw ();
  freopen ("/dev/null", "w", stderr);/* Toss the standard error output */

 /* Make sure the place is clean */
  clear ();

 /* Set up the top-level windows */
 /* First line is the header */
  displayw = subwin (stdscr, 12, 0, 2, 0);/* Lines 2-13 */
  scrollok (displayw, TRUE);

  queryw = subwin (stdscr, 1, 0, 15, 0);/* Line 15 */
  scrollok (queryw, TRUE);

  dataw = subwin (stdscr, 5, 0, 17, 0);/* Lines 17-21 */

 /* Set up the data windows */
  fnamew = subwin (stdscr, 1, 0, 17, DESC_WIDTH);
  midw = subwin (stdscr, 1, 0, 18, DESC_WIDTH);
  lnamew = subwin (stdscr, 1, 0, 19, DESC_WIDTH);
  idw = subwin (stdscr, 1, 0, 20, DESC_WIDTH);
  loginw = subwin (stdscr, 1, 0, 21, DESC_WIDTH);

}

/* Clear and restore the display */
reset_display () {
  clear ();

 /* Put back the borders */
  make_border (1);
  make_border (14);
  make_border (16);
  make_border (22);


 /* Put in the window dressing */
  wmove (dataw, 0, 0);
  waddstr (dataw, "First Name:\n");
  waddstr (dataw, "Middle Initial:\n");
  waddstr (dataw, "Family Name:\n");
  waddstr (dataw, "MIT ID #:\n\n");
  waddstr (dataw, "Username:\n");
  wclrtoeol (dataw);

 /* Set up the header */
  mvaddstr (0, (COLS - strlen (HEADER)) / 2, HEADER);
  mvaddstr (23, 0, HELP);

 /* Put it all up */
  refresh ();
}


/* Make a one-line border on line l of stdscr */
make_border (l)
int   l;
{
  int   i;

  move (l, 0);
  for (i = 0; i < COLS - 1; i++) {
    addch (BORDER_CHAR);
  }
}

/* This replaces several "useful" display functions in the old userreg */
redisp () {
  mvwprintw (fnamew, 0, 0, "%-24s", user.u_first);
  wrefresh(fnamew);
  mvwprintw (midw, 0, 0, "%-24s", user.u_mid_init);
  wrefresh(midw);
  mvwprintw (lnamew, 0, 0, "%-24s", user.u_last);
  wrefresh(lnamew);
  mvwprintw (idw, 0, 0, "%-24s", typed_mit_id);
  wrefresh(idw);
  mvwprintw (loginw, 0, 0, "%-24s", user.u_login);
  wrefresh(loginw);
}


/* Input and input_no_echo exist only to save on retyping */
input (prompt, buf, maxsize, timeout, emptyok)
char *prompt;
char *buf;
int   maxsize, timeout, emptyok;
{
  query_user (prompt, buf, maxsize, timeout, TRUE, emptyok, TRUE);
}

input_no_echo (prompt, buf, maxsize, timeout)
char *prompt;
char *buf;
int   maxsize, timeout;
{
  query_user (prompt, buf, maxsize, timeout, FALSE, FALSE, TRUE);
}


/* make the user press any key to continue */
wait_for_user ()
{
    char buf[BUFSIZ];

    redisp();
    query_user ("Press RETURN or ENTER to continue", buf, 1,
		15 * 60, FALSE, TRUE, FALSE);
}


/* Gets input through the query buffer */
/* Exit(1)'s on read errors */
/* Signals SIGALRM after 'timeout' seconds */
query_user (prompt, buf, maxsize, timeout, echop, emptyok, valuep)
char *prompt;
char *buf;
int   maxsize, timeout;
int echop, emptyok, valuep;
{
  int  c;
  int   i;
  struct itimerval it;

retry:
  /* Set up interval timer */
  it.it_interval.tv_sec = 0;
  it.it_interval.tv_usec = 0;
  it.it_value.tv_sec = timeout;
  it.it_value.tv_usec = 0;
  setitimer (ITIMER_REAL, &it, (struct itimerval *) 0);

  /* Erase the query window and put up a prompt */
  werase (queryw);
  mvwaddstr (queryw, 0, 0, prompt);
  waddch (queryw, ' ');		/* Put in a space, as Blox does */
  wrefresh (queryw);

  i = 0;
  while ((c = getchar ()) != '\r' && c != '\n') {
   switch (c) {
     case '\025': 		/* Ctl-U */
	goto retry;
      case EOF:
	/* We're in raw mode, so EOF means disaster */
	exit(1);
	break;
    delchar:
      case '\177': 		/* Delete */
      case '\010': 		/* Backspace */
	if (i) {
	  i--;
	  if (echop) {
	    wmove (queryw, queryw -> _cury, queryw -> _curx - 1);
	    wclrtoeol (queryw);
	    wrefresh (queryw);
	  }
	}
	break;
      case '\003':		/* Ctrl-C */
	clear();
	restore_display();
	exit(0);
	break;
      default: 
	if (c >= ' ') {		/* Ignore all other control chars */
	  buf[i++] = c;
	  if (echop) {
	    waddch (queryw, c);
	    wrefresh (queryw);
	  }
	}
	break;
    }
    if (valuep && i >= maxsize) {
      wfeep();
      wprintw (displayw,
  "You are not allowed to type more than %d characters for this answer.\n",
	  maxsize-1);
      wrefresh (displayw);
      goto delchar;
    }
 }

  if (i == 0) {
      if (emptyok && valuep &&
	  (askyn("Do you really want this field left blank (y/n)? ") == NO))
	goto retry;
      if (!emptyok) {
	  wprintw(displayw, "You must enter something here.\n");
	  wrefresh(displayw);
	  goto retry;
      }
  }
    

  /* Input is complete so disable interval timer. */
  it.it_interval.tv_sec = 0;
  it.it_interval.tv_usec = 0;
  it.it_value.tv_sec = 0;
  it.it_value.tv_usec = 0;
  setitimer (ITIMER_REAL, &it, (struct itimerval *) 0);

  buf[i] = '\0';		/* Put a null on the end */

  werase (queryw);		/* Clean up the query window */
  wrefresh (queryw);

  return;			/* And get out of here. */
}

int
askyn(prompt)
     char *prompt;
{
  int ypos, xpos;
  int answer;
  struct itimerval it;
  int c;

 start:
  werase (queryw);
  mvwaddstr (queryw, 0, 0, prompt);
  wrefresh (queryw);

  xpos = queryw->_curx;
  ypos = queryw->_cury;
  answer = 2;			/* No answer. */
  
  /* Reset interval timer for y/n question. */
  it.it_interval.tv_sec = 0;
  it.it_interval.tv_usec = 0;
  it.it_value.tv_sec = YN_TIMEOUT;
  it.it_value.tv_usec = 0;
  setitimer (ITIMER_REAL, &it, (struct itimerval *) 0);
    
  while ((c = getchar ()) != '\r' && c != '\n') {	/* Wait for CR. */
      switch (c) {
      case 'n':			/* No. */
      case 'N':
	wmove(queryw, ypos, xpos);
	wclrtoeol(queryw);
	waddstr(queryw, "no");
	wrefresh(queryw);
	answer = NO;
	continue;
      case 'y':			/* Yes. */
      case 'Y':
	wmove(queryw, ypos, xpos);
	wclrtoeol(queryw);
	waddstr(queryw, "yes");
	wrefresh(queryw);
	answer = YES;
	continue;
      case '\177': 		/* Delete */
      case '\010': 		/* Backspace */
      case '\025': 		/* Ctl-U */
	wmove(queryw, ypos, xpos);
	wclrtoeol(queryw);
	wrefresh(queryw);
	answer = 2;		/* No answer. */
	break;
      case EOF:
	/* We're in raw mode, so EOF means disaster */
	exit(1);
	break;
      case '\003':		/* Ctrl-C */
	clear();
	restore_display();
	exit(0);
	break;
      default:			/* Ignore everything else. */
	break;
      }
    }

  if (answer == 2)		/* No answer. */
    { display_text_line(0);
      display_text_line("Please answer y or n.");
      goto start;
    }
  
  return(answer);
}

/* Display_text_line puts up a line of text in the display window */
/* Special case: if line is 0, clear the display area */
display_text_line (line)
char *line;
{
  if (line) {
    waddstr (displayw, line);
    waddch (displayw, '\n');
    wrefresh (displayw);
  }
  else {
    werase (displayw);
  }
  wrefresh (displayw);
}

/* Display_text displays a canned message from a file */
display_text (filename)
char *filename;
{
  FILE * fp;
  char  buf[100];

  werase (displayw);
  if ((fp = fopen (filename, "r")) == NULL) {
    wprintw (displayw, "Can't open file %s for reading.\n", filename);
    return;
  }

  while (fgets (buf, 100, fp)) {
  /* get rid of the newline */
    buf[strlen (buf) - 1] = 0;
    display_text_line (buf);
  }

  fclose (fp);
}

/* Clear_display wipes the display and turns off curses */
restore_display()
{
  clear();
  refresh();
  noraw();
  echo();
  endwin();
}

timer_on()
{
  struct itimerval it;

  it.it_interval.tv_sec = 0;
  it.it_interval.tv_usec = 0;
  it.it_value.tv_sec = TIMER_TIMEOUT;
  it.it_value.tv_usec = 0;
  setitimer (ITIMER_REAL, &it, (struct itimerval *) 0);
}

timer_off()
{
  struct itimerval it;

  it.it_interval.tv_sec = 0;
  it.it_interval.tv_usec = 0;
  it.it_value.tv_sec = 0;
  it.it_value.tv_usec = 0;
  setitimer (ITIMER_REAL, &it, (struct itimerval *) 0);
}


wfeep()
{
    char buf = '\007';
    write(1, &buf, 1);
}
