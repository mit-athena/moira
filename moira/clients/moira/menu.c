/*
 * Generic menu system module.
 *
 * Basically, we define an enormous tree structure which represents the
 * menu.  Some extra pieces (ml_command, ma_doc) get thrown in so we can
 * also use the structure for a command-based system.
 *
 * By making the menu descriptions so general, we can ease porting to just
 * about anything.
 */

#include <stdio.h>
#include <curses.h>
#include "menu.h"

#define MAX(A,B) ((A) > (B) ? (A) : (B))
#define MIN(A,B) ((A) < (B) ? (A) : (B))

#define MIN_INPUT 2		/* Minimum number of lines for input window */

/* Structure for holding current displayed menu */
struct menu_screen {
    WINDOW *ms_screen;		/* Window for this menu */
    WINDOW *ms_title;		/* Title subwindow */
    WINDOW *ms_menu;		/* Menu subwindow */
    WINDOW *ms_input;		/* Input subwindow */
    int ms_input_y;		/* Input subwindow reference coordinate */
} *cur_ms;

/*
 * Start_menu takes a menu as an argument.  It initializes curses u.s.w.,
 * and a quit in any submenu should unwind back to here.  (it might not,
 * if user functions which run their own menus don't cooperate.)
 * Start_menu should only be called once, at the start of the program.
 */
Start_menu(m)
Menu *m;
{
    if(initscr() == ERR) {
	fputs("Can't initialize curses!\n", stderr);
	exit(1);
    }
    raw();			/* We parse & print everything ourselves */
    noecho();
    Do_menu(m);			/* Run the menu */
    endwin();
}

/*
 * Create a new menu screen template with the specified menu length
 * and return it.
 */
struct menu_screen *make_ms(length)
int length;
{
    struct menu_screen *ms;
    char *malloc();

    if(MAX_TITLE + length + MIN_INPUT > LINES) {
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
    wmove(ms->ms_input, 0, 0);
    wclear(ms->ms_screen);

    return(ms);
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
    free(ms);
}

/*
 * This guy actually puts up the menu
 */
int Do_menu(m)
Menu *m;
{
    struct menu_screen *my_ms, *old_cur_ms;
    char argvals[MAX_ARGC][MAX_ARGLEN];	/* This is where args are stored */
    char *argv[MAX_ARGC];
    int line;
    int argc;
    int quitflag;

    /* Initialize argv */
    for(argc = 0; argc < MAX_ARGC; argc++) argv[argc] = argvals[argc];

    /* Entry function gets called with old menu_screen still current */
    if(m->m_entry != NULLFUNC) m->m_entry(m);

    /* Get a menu_screen */
    old_cur_ms = cur_ms;
    cur_ms = my_ms = make_ms(m->m_length + 2);

    /* Now print the title and the menu */
    wmove(my_ms->ms_title, 0, MAX(0, (COLS - strlen(m->m_title))>>1));
    wstandout(my_ms->ms_title);
    waddstr(my_ms->ms_title, m->m_title);
    wstandend(my_ms->ms_title);

    for(line = 0; line < m->m_length; line++) {
	wmove(my_ms->ms_menu, line, 0);
	wprintw(my_ms->ms_menu, "%2d. (%-8s) %s.", line+1,
		m->m_lines[line].ml_command,
		m->m_lines[line].ml_doc);
    }
    wmove(my_ms->ms_menu, line++, 0);
    waddstr(my_ms->ms_menu, " r. (return  ) Return to previous menu.");
    wmove(my_ms->ms_menu, line, 0);
    waddstr(my_ms->ms_menu, " q. (quit    ) Quit.");

    refresh_ms(my_ms);
    for(;;) {
	/* This will be set by a return val from func or submenu */
	quitflag = DM_NORMAL;
	/* Get a command */
	Prompt_input("Command: ", argvals[0], sizeof(argvals[0]));
	/* See if it matches anything (this is kind of dumb, for now) */
	if((line = atoi(argvals[0])) > 0 && line <= m->m_length) {
	    line--;
	    /* Send the command into the argv */
	    strcpy(argvals[0], m->m_lines[line].ml_command);
	    /* Show that we're working on it */
	    Put_message(m->m_lines[line].ml_doc);
	    /* Get arguments, if any */
	    for(argc = 1; argc < m->m_lines[line].ml_argc; argc++) {
		Prompt_input(m->m_lines[line].ml_args[argc].ma_prompt,
			     argvals[argc], sizeof(argvals[argc]));
	    }
	    if(m->m_lines[line].ml_function != NULLFUNC) {
		/* If it's got a function, call it */
		quitflag = m->m_lines[line].ml_function(argv, argc);
	    } else if(m->m_lines[line].ml_submenu != NULLMENU) {
		/* Else see if it is a submenu */
		quitflag = Do_menu(m->m_lines[line].ml_submenu);
	    } else {
		/* If it's got neither, something is wrong */
		Put_message("*INTERNAL ERROR: NO FUNCTION OR MENU FOR LINE*");
	    }
	} else if(!strcmp(argv[0], "r") || !strcmp(argv[0], "q")) {
	    /* here if it's either return or quit */
	    cur_ms = old_cur_ms;
	    destroy_ms(my_ms);
	    if(m->m_exit != NULLFUNC) m->m_exit(m);
	    return(*argv[0] == 'r' ? DM_NORMAL : DM_QUIT);
	} else {
	    /* Couldn't find it, give up */
	    Put_message("Command not recognized");
	    continue;		/* No good */
	}
	if(quitflag == DM_QUIT) {
	    cur_ms = old_cur_ms;
	    destroy_ms(my_ms);
	    if(m->m_exit != NULLFUNC) m->m_exit(m);
	    return(DM_QUIT);
	}
    }
}

/* Prompt the user for input in the input window of cur_ms */
Prompt_input(prompt, buf, buflen)
char *prompt;
char *buf;
int buflen;
{
    int c;
    char *p;
    int y, x, oldx;

    waddstr(cur_ms->ms_input, prompt);
    getyx(cur_ms->ms_input, y, x);
    oldx = x;
    for (p = buf; p - buf < buflen; ) {
	wmove(cur_ms->ms_input, y, x);
	wclrtoeol(cur_ms->ms_input);
	refresh_ms(cur_ms);
	c = getchar();
	switch (c) {
	case 'L' & 037:
	    touchwin(cur_ms->ms_screen);
	    break;
	case '\n': case '\r':
	    waddch(cur_ms->ms_input, '\n');
	    *p = '\0';
	    return;
	case '\b':
	case '\177':
	    if (p > buf) {
		p--;
		x--;
	    }
	    break;
	case 'U' & 037: case '\007': case '\033':
	    x = oldx;
	    break;
	default:
	    waddch(cur_ms->ms_input, c);
	    *p++ = c;
	    x++;
	    break;
	}
    }
}

/* Print a message in the input window of cur_ms */
Put_message(msg)
char *msg;
{
    wprintw(cur_ms->ms_input, "%s\n", msg);
    refresh_ms(cur_ms);
}

/* Refresh a menu_screen onto the real screen */
refresh_ms(ms)
struct menu_screen *ms;
{
    int y, x;

    getyx(ms->ms_input, y, x);
    wmove(ms->ms_screen, y + ms->ms_input_y, x);
    touchwin(ms->ms_screen);
    wrefresh(ms->ms_screen);
}
