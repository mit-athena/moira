/*
 * Copyright 1987 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file
 * "mit-copyright.h".
 *
 * $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/menu.c,v $
 * $Author: mar $
 * $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/menu.c,v 1.26 1990-01-19 18:02:30 mar Exp $
 *
 * Generic menu system module.
 *
 * Basically, we define an enormous tree structure which represents the
 * menu.  Some extra pieces (ml_command, ma_doc) get thrown in so we can
 * also use the structure for a command-based system.
 *
 * By making the menu descriptions so general, we can ease porting to just
 * about anything.
 */

#ifndef lint
static char rcsid_menu_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/menu.c,v 1.26 1990-01-19 18:02:30 mar Exp $";

#endif lint

#include <mit-copyright.h>
#include <sys/types.h>
#include <stdio.h>
#include <signal.h>
#include <curses.h>
#include <ctype.h>
#include <strings.h>
#include <varargs.h>
#include <com_err.h>
#include "menu.h"

#ifndef __STDC__
#define const
#endif

#define MAX(A,B)	((A) > (B) ? (A) : (B))
#define MIN(A,B)	((A) < (B) ? (A) : (B))
#define CTL(ch)		((ch) & 037)

#define MIN_INPUT 2		/* Minimum number of lines for input window */

extern FILE *fdopen();
extern int getpid();
extern char *calloc();
extern char *whoami;

FILE *log_file = (FILE *) NULL;		/* file stream of log file */
int more_flg = 1;

/* Structure for holding current displayed menu */
struct menu_screen {
    WINDOW *ms_screen;		/* Window for this menu */
    WINDOW *ms_title;		/* Title subwindow */
    WINDOW *ms_menu;		/* Menu subwindow */
    WINDOW *ms_input;		/* Input subwindow */
    int ms_input_y;		/* Input subwindow reference coordinate */
}          *cur_ms;

#define NULLMS ((struct menu_screen *) 0)

Menu *top_menu;			/* Root for command search */

/*
 * Hook function to cause error messages to be printed through
 * curses instead of around it.
 */

void
menu_com_err_hook(who, code, fmt, args)
    const char *who;
    long code;
    const char *fmt;
    va_list args;
{
    char buf[BUFSIZ], *cp;

    FILE _strbuf;

    (void) strcpy(buf, who);
    for (cp = buf; *cp; cp++);
    *cp++ = ':';
    *cp++ = ' ';
    if (code) {
	(void) strcpy(cp, error_message(code));
	while (*cp)
	    cp++;
    }
    _strbuf._flag = _IOWRT + _IOSTRG;
    _strbuf._ptr = cp;
    _strbuf._cnt = BUFSIZ - (cp - buf);
    _doprnt(fmt, args, &_strbuf);
    (void) putc('\0', &_strbuf);
    Put_message(buf);
}

/*
 * Start_menu takes a menu as an argument.  It initializes curses u.s.w.,
 * and a quit in any submenu should unwind back to here.  (it might not,
 * if user functions which run their own menus don't cooperate.)
 * Start_menu should only be called once, at the start of the program.
 */
Start_menu(m)
    Menu *m;
{
    struct menu_screen *make_ms();
#ifdef __STDC__
    register void (*old_hook)(const char *, long, const char *, va_list) = set_com_err_hook(menu_com_err_hook);
#else
    register void (*old_hook)() = set_com_err_hook(menu_com_err_hook);
#endif
    
    if (initscr() == ERR) {
	fputs("Can't initialize curses!\n", stderr);
	Start_no_menu(m);
    } else {
	(void) raw();		/* We parse & print everything ourselves */
	(void) noecho();
	cur_ms = make_ms(0);	/* So we always have some current */
				/* menu_screen */ 
	/* Run the menu */
	(void) Do_menu(m, -1, (char **) NULL);
    }
    (void) set_com_err_hook(old_hook);
    Cleanup_menu();
}

Cleanup_menu()
{
    if (cur_ms) {
	(void) wclear(cur_ms->ms_screen);
	(void) wrefresh(cur_ms->ms_screen);
    }
    endwin();
}
    

/* Like Start_menu, except it doesn't print menus and doesn't use curses */
Start_no_menu(m)
    Menu *m;
{
    cur_ms = NULLMS;
    COLS = 80;
    /* Run the menu */
    (void) Do_menu(m, -1, (char **) NULL);
}

/*
 * Create a new menu screen template with the specified menu length
 * and return it.
 */
struct menu_screen *
make_ms(length)
    int length;
{
    struct menu_screen *ms;
    char *malloc();

    if (MAX_TITLE + length + MIN_INPUT > LINES) {
	fputs("Menu too big!\n", stderr);
	exit(2);
    }

    ms = (struct menu_screen *) malloc(sizeof(struct menu_screen));

    ms->ms_screen = newwin(0, 0, 0, 0);
    ms->ms_title = subwin(ms->ms_screen, MAX_TITLE, 0, 0, 0);
    ms->ms_menu = subwin(ms->ms_screen,
			 length, 0, MAX_TITLE, 0);
    ms->ms_input = subwin(ms->ms_screen, 0, 0,
			  ms->ms_input_y = MAX_TITLE + length,
			  0);

    scrollok(ms->ms_input, TRUE);
    (void) wmove(ms->ms_input, 0, 0);
    (void) wclear(ms->ms_screen);

    return (ms);
}

/*
 * This routine destroys a menu_screen.
 */
destroy_ms(ms)
    struct menu_screen *ms;
{
    delwin(ms->ms_title);
    delwin(ms->ms_menu);
    delwin(ms->ms_input);
    delwin(ms->ms_screen);
    free((char *) ms);
}

/*
 * This guy actually puts up the menu
 * Note: if margc < 0, no 'r' option will be displayed (i.e., on the
 * top level menu)
 */
int 
Do_menu(m, margc, margv)
    Menu *m;
    int margc;
    char *margv[];
{
    struct menu_screen *my_ms = NULLMS, *old_cur_ms = NULLMS;
    char argvals[MAX_ARGC][MAX_ARGLEN];	/* This is where args are stored */
    char buf[MAX_ARGC * MAX_ARGLEN];
    char *argv[MAX_ARGC];
    int line;
    int i;
    struct menu_line *command, *Find_command();
    int argc;
    int quitflag, is_topmenu = (margc < 0);
    int toggle_logging();
    
    /* Entry function gets called with old menu_screen still current */
    if (m->m_entry != NULLFUNC)
	if (m->m_entry(m, margc, margv) == DM_QUIT)
	    return DM_NORMAL;

    /* The following get run only in curses mode */
    if (cur_ms != NULLMS) {
	/* Get a menu_screen */
	old_cur_ms = cur_ms;
	/* 2 is for the 2 obligatory lines; quit and toggle */
	cur_ms = my_ms = make_ms(m->m_length + 2 + (is_topmenu ? 0 : 1));

	/* Now print the title and the menu */
	(void) wclear(my_ms->ms_menu);
	(void) wmove(my_ms->ms_title, 0, MAX(0, (COLS -
						 strlen(m->m_title)) >> 1));
	(void) wstandout(my_ms->ms_title);
	(void) waddstr(my_ms->ms_title, m->m_title);
	(void) wstandend(my_ms->ms_title);

	for (line = 0; line < m->m_length; line++) {
	    int len = strlen(m->m_lines[line].ml_command);
	    if (len > 12) len=12;
	    
	    (void) wmove(my_ms->ms_menu, line, 0);
	    
	    (void) wprintw(my_ms->ms_menu, "%2d. (%s)%*s %s.", line + 1,
			   m->m_lines[line].ml_command,
			   12-len, "",
			   m->m_lines[line].ml_doc);
	}
	(void) wmove(my_ms->ms_menu, line++, 0);
	if (!is_topmenu) {
	    (void) waddstr(my_ms->ms_menu,
			   " r. (return)       Return to previous menu.");
	    (void) wmove(my_ms->ms_menu, line++, 0);
	}
	(void) waddstr(my_ms->ms_menu,
			   " t. (toggle)       Toggle logging on and off.");
	(void) wmove(my_ms->ms_menu, line, 0);
	(void) waddstr(my_ms->ms_menu, " q. (quit)         Quit.");
    } else {
	Put_message(m->m_title);
	for (line = 0; line < m->m_length; line++) {
	    sprintf(buf, "%2d. (%s)%*s %s.", line + 1,
		    m->m_lines[line].ml_command,
		    12 - strlen(m->m_lines[line].ml_command), "",
		    m->m_lines[line].ml_doc);
	    Put_message(buf);
	}
	if (!is_topmenu)
	    Put_message(" r. (return)       Return to previous menu.");
	Put_message(" t. (toggle)       Toggle logging on and off.");
	Put_message(" q. (quit)         Quit.");
	Put_message(" ?.                Print this information.");
    }

    for (;;) {
	/* This will be set by a return val from func or submenu */
	quitflag = DM_NORMAL;
	/* This is here because we may be coming from another menu */
	if (cur_ms != NULL)
	    touchwin(my_ms->ms_screen);
	/* Get a command */
	if (!Prompt_input("Command: ", buf, sizeof(buf))) {
	    if (cur_ms == NULLMS &&
		feof(stdin))
	      sprintf(buf, "quit");
	    else
	      continue;
	}
	/* Parse it into the argument list */
	/* If there's nothing there, try again */
	/* Initialize argv */
	for (argc = 0; argc < MAX_ARGC; argc++)
	    argv[argc] = argvals[argc];

	if ((argc = Parse_words(buf, argv, MAX_ARGLEN)) == 0)
	    continue;
	if ((line = atoi(argv[0])) > 0 && line <= m->m_length) {
	    command = &m->m_lines[line - 1];
	}
	else if ((!is_topmenu &&
		  (!strcmp(argv[0], "r") || !strcmp(argv[0], "return")))
		 || !strcmp(argv[0], "q") || !strcmp(argv[0], "quit")) {
		 
	    /* here if it's either return or quit */
	    if (cur_ms != NULLMS) {
		cur_ms = old_cur_ms;
		destroy_ms(my_ms);
	    }
	    if (m->m_exit != NULLFUNC)
		m->m_exit(m);
	    return (*argv[0] == 'r' ? DM_NORMAL : DM_QUIT);
	}
	else if (argv[0][0] == '?') {
	    for (line = 0; line < m->m_length; line++) {
		sprintf(buf, "%2d. (%s)%*s %s.", line + 1,
			m->m_lines[line].ml_command,
			12 - strlen(m->m_lines[line].ml_command), "",
			m->m_lines[line].ml_doc);
		Put_message(buf);
	    }
	    if (!is_topmenu)
		Put_message(" r. (return)       Return to previous menu.");
	    Put_message(" t. (toggle)       Toggle logging on and off.");
	    Put_message(" q. (quit)         Quit.");
	    continue;
	}
	else if (!strcmp(argv[0], "t") || !strcmp(argv[0], "toggle")) {
	    toggle_logging(argc, argv);
	    continue;
	} 
	/* finally, try to find it using Find_command */
	else if ((command = Find_command(m, argvals[0])) ==
		 (struct menu_line *) 0) {
	    Put_message("Command not recognized");
	    continue;
	}
	/* If we got to here, command is a valid menu_line */
	/* Send the offical command name into the argv */
	(void) strcpy(argvals[0], command->ml_command);
	/* Show that we're working on it */
	Put_message(command->ml_doc);
	/* Print args that we've already got */
	for (i = 1; i < argc; i++) {
	    if (command->ml_args[i].ma_prompt == NULL)
		break;
	    (void) sprintf(buf, "%s%s", command->ml_args[i].ma_prompt,
			   argv[i]);
	    Put_message(buf);
	}
	/* Get remaining arguments, if any */
	for (; argc < command->ml_argc; argc++) {
	    if (!Prompt_input(command->ml_args[argc].ma_prompt,
			      argvals[argc], sizeof(argvals[argc])))
		goto punt_command;
	}
	if (command->ml_function != NULLFUNC) {
	    /* If it's got a function, call it */
	    quitflag = command->ml_function(argc, argv);
	}
	else if (command->ml_submenu != NULLMENU) {
	    /* Else see if it is a submenu */
	    quitflag = Do_menu(command->ml_submenu, argc, argv);
	}
	else {
	    /* If it's got neither, something is wrong */
	    Put_message("*INTERNAL ERROR: NO FUNCTION OR MENU FOR LINE*");
	}
	if (quitflag == DM_QUIT) {
	    if (cur_ms != NULLMS) {
		cur_ms = old_cur_ms;
		destroy_ms(my_ms);
	    }
	    if (m->m_exit != NULLFUNC)
		m->m_exit(m);
	    return (DM_QUIT);
	}
    punt_command:
	;
    }
}

/* Prompt the user for input in the input window of cur_ms */
int Prompt_input(prompt, buf, buflen)
    char *prompt;
    char *buf;
    int buflen;
{
    int c;
    char *p;
    int y, x, oldx, oldy;

    if (cur_ms != NULLMS) {
	more_flg = 1;
	getyx(cur_ms->ms_input, y, x);
	(void) wmove(cur_ms->ms_input, y, 0);

	touchwin(cur_ms->ms_screen);
	refresh_ms(cur_ms);
	(void) waddstr(cur_ms->ms_input, prompt);
	getyx(cur_ms->ms_input, y, x);

	oldx = x;
	oldy = y;
	p = buf;
	while(1) {
	    (void) wmove(cur_ms->ms_input, y, x);
		(void) touchwin(cur_ms->ms_screen);
	    (void) wclrtoeol(cur_ms->ms_input);
	    refresh_ms(cur_ms);
	    c = getchar() & 0x7f;
	    switch (c) {
	    case CTL('C'):
		*p = '\0';
		return 0;
	    case CTL('Z'):
		(void) kill(getpid(), SIGTSTP);
		touchwin(curscr);
		break;
	    case CTL('L'):
		(void) wclear(cur_ms->ms_input);
		(void) waddstr(cur_ms->ms_input, prompt);
		(void) touchwin(cur_ms->ms_input);
		(void) mvcur(0, COLS - 1, LINES - 1, 0);
		(void) wrefresh(curscr);
		getyx(cur_ms->ms_input, y, x);
		oldy = y;
		oldx = x;
		p = buf;
		break;

	    case '\n':
	    case '\r':
		goto end_input;
		/* these should be obtained by doing ioctl() on tty */
	    case '\b':
	    case '\177':
		if (p > buf) {
		    p--;
		    x--;
		    if (x < 0) {
			(void) wmove(cur_ms->ms_input, y, 0);
			(void) wclrtoeol(cur_ms->ms_input);
			y--;
			x = cur_ms->ms_input->_maxx-1;
		    }
		}
		break;
	    case CTL('U'):
	    case CTL('G'):
	    case CTL('['):
		x = oldx;
		y = oldy;
		p = buf;
		break;
	    default:
		/* (buflen - 1) leaves room for the \0 */
		if (isprint(c) && (p - buf < buflen - 1)) {
		    (void) waddch(cur_ms->ms_input, c);
		    *p++ = c;
		    x++;
		    if (x >= cur_ms->ms_input->_maxx) {
			x = 0;
			y++;
		    }
		} else
		    putchar(CTL('G'));
		break;
	    }
	}
    end_input:
	(void) waddch(cur_ms->ms_input, '\n');
	(void) waddch(cur_ms->ms_input, '\r');
	
	(void) wclrtoeol(cur_ms->ms_input);
	refresh_ms(cur_ms);
	*p = '\0';
	Start_paging();
	goto gotit;
    } else {
	printf("%s", prompt);
	if (gets(buf) == NULL)
	    return 0;
	Start_paging();
	goto gotit;
    }
gotit:
    strcpy(buf, strtrim(buf));
    return 1;
}

/* Prompt the user for input in the input window of cur_ms, but don't echo
   and allow some control characters */
int Password_input(prompt, buf, buflen)
    char *prompt;
    char *buf;
    int buflen;
{
    int c;
    char *p;
    int y, x, oldx;

    if (cur_ms != NULLMS) {
	more_flg = 1;
	getyx(cur_ms->ms_input, y, x);
	(void) wmove(cur_ms->ms_input, y, 0);

	touchwin(cur_ms->ms_screen);
	refresh_ms(cur_ms);
	(void) waddstr(cur_ms->ms_input, prompt);
	getyx(cur_ms->ms_input, y, x);

	oldx = x;
	for (p = buf; p - buf < buflen;) {
	    (void) wmove(cur_ms->ms_input, y, x);
	    (void) wclrtoeol(cur_ms->ms_input);
	    refresh_ms(cur_ms);
	    c = getchar() & 0x7f;
	    switch (c) {
	    case CTL('C'):
		return 0;
	    case CTL('Z'):
		(void) kill(getpid(), SIGTSTP);
		touchwin(curscr);
		break;
	    case CTL('L'):
		(void) wclear(cur_ms->ms_input);
		(void) waddstr(cur_ms->ms_input, prompt);
		refresh_ms(cur_ms);
		(void) mvcur(0, COLS - 1, LINES - 1, 0);
		(void) wrefresh(curscr);
		getyx(cur_ms->ms_input, y, x);
		break;
	    case '\n':
	    case '\r':
		(void) waddch(cur_ms->ms_input, '\n');
		(void) waddch(cur_ms->ms_input, '\r');

		(void) wclrtoeol(cur_ms->ms_input);
		refresh_ms(cur_ms);
		*p = '\0';
		Start_paging();
		return 1;
	    case '\b':
	    case '\177':
		if (p > buf) {
		    p--;
		    x--;
		}
		break;
	    case CTL('U'):
		x = oldx;
		p = buf;
		break;
	    default:
		*p++ = c;
		break;
	    }
	}
    }
    else {
	struct sgttyb ttybuf, nttybuf;
	printf("%s", prompt);
	/* turn off echoing */
	(void) ioctl(0, TIOCGETP, (char *)&ttybuf);
	nttybuf = ttybuf;
	nttybuf.sg_flags &= ~ECHO;
	(void)ioctl(0, TIOCSETP, (char *)&nttybuf);
	if (gets(buf) == NULL) {
	    (void) ioctl(0, TIOCSETP, (char *)&ttybuf);
	    putchar('\n');
	    return 0;
	}
	putchar('\n');
	(void) ioctl(0, TIOCSETP, (char *)&ttybuf);
	Start_paging();
	return 1;
    }
    return 0;
}

int lines_left;

/* Start paging */
/* This routine will cause the most recently put message to be the
   one at the top of the screen when a ---More--- prompt is displayed */
Start_paging()
{
    if (cur_ms != NULLMS) {
	lines_left = LINES - cur_ms->ms_input_y - 1;
    }
    else {
	lines_left = 23;
    }
}

/* Turn off paging */
Stop_paging()
{
    lines_left = -1;
}

/* Print a message in the input window of cur_ms.  */
Put_message(msg)
char *msg;
{
    char *copy, *line, *s;

    copy = (char *) malloc((u_int)COLS);
    s = line = msg;
    if (log_file) {	/* if we're doing logging; we assume that the */
			/* file has already been opened. */
	(void) fprintf(log_file, "%s\n", msg);
	fflush(log_file);
    } 

    while(*s++) {
	if (s - line >= COLS-1) {
	    (void) strncpy(copy, line, COLS-1);
	    line += COLS-1;
	} else if (*s == '\n') {
	    *s = '\0';
	    (void) strcpy(copy, line);
	    line = ++s;
	} else
	    continue;
	Put_line(copy);
    }
    Put_line(line);
    free(copy);
}

/* Will be truncated to COLS characters.  */
Put_line(msg)
char *msg;
{
    int y, x, i;
    char *msg1, chr;

    if (!more_flg)
	return;

    if (lines_left >= 0) {
	if (--lines_left == 0) {
	    /* Give the user a more prompt */
	    if (cur_ms != NULLMS) {
		(void) wstandout(cur_ms->ms_input);
		(void) wprintw(cur_ms->ms_input, "---More---");
		(void) wstandend(cur_ms->ms_input);
		refresh_ms(cur_ms);
		chr = getchar() & 0x7f;/* We do care what it is */
		if (chr == 'q' || chr == 'Q') {
		    more_flg = 0;
		    return;
		}
		getyx(cur_ms->ms_input, y, x);
		/* x is a bitbucket; avoid lint problems */
		x=x;
		(void) wmove(cur_ms->ms_input, y, 0);
		(void) wclrtoeol(cur_ms->ms_input);
	    }
	    else {
		printf("---More (hit return)---");
		getchar();
	    }
	    Start_paging();	/* Reset lines_left */
	}
    }

    if (cur_ms != NULLMS) {
	msg1 = (char *) calloc((u_int)COLS, 1);
	(void) strncpy(msg1, msg, COLS-1);
	for (i = strlen(msg1); i < COLS - 1; i++)
	    msg1[i] = ' ';
	(void) wprintw(cur_ms->ms_input, "%s\n", msg1);
/*	refresh_ms(cur_ms); */
    }
    else {
	puts(msg);
    }
}

/* Refresh a menu_screen onto the real screen */
refresh_ms(ms)
    struct menu_screen *ms;
{
    int y, x;

    getyx(ms->ms_input, y, x);
    (void) wmove(ms->ms_screen, y + ms->ms_input_y, x);
    (void) wrefresh(ms->ms_screen);
}

/* Parse buf into a list of words, which will be placed in strings specified by
   argv.  Space for these strings must have already been allocated.
   Only the first n characters of each word will be copied */
Parse_words(buf, argv, n)
    char *buf;
    char *argv[];
int n;

{
    char *start, *end;		/* For sausage machine */
    int argc;

    start = buf;
    for (argc = 0; argc < MAX_ARGC; argc++) {
	while (isspace(*start))
	    start++;		/* Kill whitespace */
	if (*start == '\0')
	    break;		/* Nothing left */
	/* Now find the end of the word */
	for (end = start; *end != '\0' && !isspace(*end); end++);
	(void) strncpy(argv[argc], start, MIN(end - start, n));	/* Copy it */
	argv[argc][MIN(end - start, n - 1)] = '\0';	/* Terminate */
	start = end;
    }
    return (argc);
}

/* This is the internal form of Find_command, which recursively searches
   for a menu_line with command command in the specified menu */
/* It will search to a maximum depth of d */
struct menu_line *
find_command_from(c, m, d)
    char *c;
    struct menu *m;
    int d;
{
    int line;
    struct menu_line *maybe;

    if (d < 0)
	return ((struct menu_line *) 0);	/* Too deep! */
    for (line = 0; line < m->m_length; line++) {
	if (!strcmp(c, m->m_lines[line].ml_command)) {
	    return (&m->m_lines[line]);
	}
    }
    for (line = 0; line < m->m_length; line++) {
	if (m->m_lines[line].ml_submenu != NULLMENU  &&
	    (maybe = find_command_from(c, m->m_lines[line].ml_submenu, d - 1))
		!= (struct menu_line *) 0) {
	    return (maybe);
	}
    }
    /* If we got to here, nothing works */
    return ((struct menu_line *) 0);
}

/* Find_command searches down the current menu tree */
/* And returns a pointer to a menu_line with the specified command name */
/* It returns (struct menu_line *) 0 if none is found */
struct menu_line *
Find_command(m, command)
    Menu *m;
    char *command;
{
    if (m == NULLMENU) {
	return ((struct menu_line *) 0);
    }
    else {
	return (find_command_from(command, m, MAX_MENU_DEPTH));
    }
}

/*ARGSUSED*/
int
toggle_logging(argc, argv)
    int argc;
    char *argv[];
{
    int pid;
    char buf[BUFSIZ];
    
    if (log_file == (FILE *) NULL) {
	pid = getpid();
	if (!whoami) {
	    Put_message("I've lost my SENSE of DIRECTION!  I have no IDEA who I AM!");
	    Put_message("My God... You've turned him into a DEMOCRAT!!");
	    Put_message("		-- Doonesbury");
	    Put_message("");
	    Put_message("translation:  your log file can be found in \"/usr/tmp/a.out.pid\".");
	    whoami = "a.out";
	    (void) sprintf(buf, "/usr/tmp/%s-log.%d", whoami, pid);
	}
	else
	    (void) sprintf(buf, "/usr/tmp/%s-log.%d", whoami, pid);

	/* open the file */
	log_file = fopen(buf,"a");

	if (log_file == (FILE *) NULL)
	    Put_message("Open of log file failed.  Logging is not on.");
	else
	    Put_message("Log file successfully opened.");
    } 
    else { /* log_file is a valid pointer; turn off logging. */
	(void) fflush(log_file);
	(void) fclose(log_file);
	log_file = (FILE *) NULL;
	Put_message("Logging off.");
    }
    return(DM_NORMAL);
} 
