/* $Id: menu.c,v 1.59 2000-03-16 22:23:49 zacheiss Exp $
 *
 * Generic menu system module.
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include "menu.h"

#include <ctype.h>
#ifdef HAVE_CURSES
#include <curses.h>
#endif
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#include <process.h>
#ifdef getchar
#undef getchar
#endif
#define getchar() _getch()
#define getpid _getpid
#endif /* _WIN32 */

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/menu.c,v 1.59 2000-03-16 22:23:49 zacheiss Exp $");

#ifdef MAX
#undef MAX
#undef MIN
#endif
#define MAX(A, B)	((A) > (B) ? (A) : (B))
#define MIN(A, B)	((A) < (B) ? (A) : (B))
#define CTL(ch)		((ch) & 037)

#define MIN_INPUT 2		/* Minimum number of lines for input window */

extern int interrupt;		/* will be set if ^C is received */
extern char *whoami;

FILE *log_file = (FILE *) NULL;		/* file stream of log file */
int more_flg = 1;

/* Structure for holding current displayed menu */
struct menu_screen {
#ifdef HAVE_CURSES
  WINDOW *ms_screen;		/* Window for this menu */
  WINDOW *ms_title;		/* Title subwindow */
  WINDOW *ms_menu;		/* Menu subwindow */
  WINDOW *ms_input;		/* Input subwindow */
#endif /* HAVE_CURSES */
  int ms_input_y;		/* Input subwindow reference coordinate */
} *cur_ms;

#ifndef HAVE_CURSES
int COLS;
#endif

#define NULLMS ((struct menu_screen *) 0)

Menu *top_menu;			/* Root for command search */
int parsed_argc;		/* used by extern routines to get additional */
char **parsed_argv;		/*   comand line input */

int Parse_words(char *buf, char *argv[], int n);
void refresh_ms(struct menu_screen *ms);
void Put_line(char *msg);
void menu_com_err_hook(const char *who, long code,
		       const char *fmt, va_list ap);
struct menu_screen *make_ms(int length);
void destroy_ms(struct menu_screen *ms);
struct menu_line *find_command_from(char *c, struct menu *m, int d);
struct menu_line *Find_command(Menu *m, char *command);
int toggle_logging(int argc, char *argv[]);

/*
 * Hook function to cause error messages to be printed through
 * curses instead of around it.
 */

void menu_com_err_hook(const char *who, long code, const char *fmt, va_list ap)
{
  char buf[BUFSIZ], *cp;

  if (who)
    {
      strcpy(buf, who);
      for (cp = buf; *cp; cp++)
	;
      *cp++ = ':';
      *cp++ = ' ';
    }
  else
    {
      cp = buf;
      *cp = '\0';
    }
  if (code)
    {
      strcpy(cp, error_message(code));
      while (*cp)
	cp++;
    }
  vsprintf(cp, fmt, ap);
  Put_message(buf);
}

#ifdef HAVE_CURSES
/*
 * Start_menu takes a menu as an argument.  It initializes curses u.s.w.,
 * and a quit in any submenu should unwind back to here.  (it might not,
 * if user functions which run their own menus don't cooperate.)
 * Start_menu should only be called once, at the start of the program.
 */
void Start_menu(Menu *m)
{
  void (*old_hook)(const char *, long, const char *, va_list) =
    set_com_err_hook(menu_com_err_hook);
#ifdef CURSES_HAS_NEWTERM
  SCREEN *scrn = newterm(NULL, stdout, stdin);
#else
  WINDOW *scrn = initscr();
#endif
  if (!scrn)
    {
      fputs("Can't initialize curses!\nReverting to -nomenu mode\n\n", stderr);
      Start_no_menu(m);
    }
  else
    {
#ifdef CURSES_HAS_NEWTERM
      set_term(scrn);
      endwin();
      initscr();
#endif
      raw();		/* We parse & print everything ourselves */
      noecho();
      cur_ms = make_ms(0);	/* So we always have some current */
				/* menu_screen */
      /* Run the menu */
      Do_menu(m, -1, NULL);
      Cleanup_menu();
    }
  set_com_err_hook(old_hook);
}

void Cleanup_menu(void)
{
  if (cur_ms)
    {
      wclear(cur_ms->ms_screen);
      wrefresh(cur_ms->ms_screen);
      endwin();
    }
}

/*
 * Create a new menu screen template with the specified menu length
 * and return it.
 */
struct menu_screen *make_ms(int length)
{
  struct menu_screen *ms;

  if (MAX_TITLE + length + MIN_INPUT > LINES)
    {
      fputs("Menu too big!\n", stderr);
      exit(2);
    }

  ms = malloc(sizeof(struct menu_screen));

  ms->ms_screen = newwin(0, 0, 0, 0);
  ms->ms_title = subwin(ms->ms_screen, MAX_TITLE, 0, 0, 0);
  ms->ms_menu = subwin(ms->ms_screen, length, 0, MAX_TITLE, 0);
  ms->ms_input = subwin(ms->ms_screen, 0, 0,
			ms->ms_input_y = MAX_TITLE + length,
			0);

  scrollok(ms->ms_input, TRUE);
  wmove(ms->ms_input, 0, 0);
  wclear(ms->ms_screen);

  return ms;
}

/*
 * This routine destroys a menu_screen.
 */
void destroy_ms(struct menu_screen *ms)
{
  delwin(ms->ms_title);
  delwin(ms->ms_menu);
  delwin(ms->ms_input);
  delwin(ms->ms_screen);
  free(ms);
}
#endif /* HAVE_CURSES */

/* Like Start_menu, except it doesn't print menus and doesn't use curses */
void Start_no_menu(Menu *m)
{
  cur_ms = NULLMS;
  COLS = 80;
  /* Run the menu */
  Do_menu(m, -1, NULL);
}

/*
 * This guy actually puts up the menu
 * Note: if margc < 0, no 'r' option will be displayed (i.e., on the
 * top level menu)
 */
int Do_menu(Menu *m, int margc, char *margv[])
{
  struct menu_screen *my_ms = NULLMS, *old_cur_ms = NULLMS;
  char argvals[MAX_ARGC][MAX_ARGLEN];	/* This is where args are stored */
  char buf[MAX_ARGC * MAX_ARGLEN];
  char *argv[MAX_ARGC];
  int line;
  int i;
  struct menu_line *command;
  int argc;
  int quitflag, is_topmenu = (margc < 0);

  /* Entry function gets called with old menu_screen still current */
  if (m->m_entry != NULLFUNC)
    {
      if (m->m_entry(m, margc, margv) == DM_QUIT)
	return DM_NORMAL;
      if (parsed_argc > 0)
	{
	  margc = parsed_argc + 1;
	  margv = --parsed_argv;
	}
      else
	{
	  margc--;
	  margv++;
	}
    }

  parsed_argc = 0;

#ifdef HAVE_CURSES
  /* The following get run only in curses mode */
  if (cur_ms != NULLMS)
    {
      /* Get a menu_screen */
      old_cur_ms = cur_ms;
      /* 2 is for the 2 obligatory lines; quit and toggle */
      cur_ms = my_ms = make_ms(m->m_length + 2 + (is_topmenu ? 0 : 1));

      /* Now print the title and the menu */
      wclear(my_ms->ms_screen);
      wrefresh(my_ms->ms_screen);
      wmove(my_ms->ms_title, 0, MAX(0, (COLS - (int)strlen(m->m_title)) >> 1));
      wstandout(my_ms->ms_title);
      waddstr(my_ms->ms_title, m->m_title);
      wstandend(my_ms->ms_title);

      for (line = 0; line < m->m_length; line++)
	{
	  int len = strlen(m->m_lines[line].ml_command);
	  if (len > 12)
	    len = 12;

	  wmove(my_ms->ms_menu, line, 0);

	  wprintw(my_ms->ms_menu, "%2d. (%s)%*s %s.", line + 1,
		  m->m_lines[line].ml_command, 12 - len, "",
		  m->m_lines[line].ml_doc);
	}
      wmove(my_ms->ms_menu, line++, 0);
      if (!is_topmenu)
	{
	  waddstr(my_ms->ms_menu,
		  " r. (return)       Return to previous menu.");
	  wmove(my_ms->ms_menu, line++, 0);
	}
      waddstr(my_ms->ms_menu, " t. (toggle)       Toggle logging on and off.");
      wmove(my_ms->ms_menu, line, 0);
      waddstr(my_ms->ms_menu, " q. (quit)         Quit.");
    }
  else
#endif /* HAVE_CURSES */
    {
      Put_message(m->m_title);
      for (line = 0; line < m->m_length; line++)
	{
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

  for (;;)
    {
      /* This will be set by a return val from func or submenu */
      quitflag = DM_NORMAL;
#ifdef HAVE_CURSES
      /* This is here because we may be coming from another menu */
      if (cur_ms != NULL)
	{
	  touchwin(my_ms->ms_screen);
	  wrefresh(my_ms->ms_screen);
	}
#endif /* HAVE_CURSES */
      if (margc > 1)
	{
	  /* Initialize argv */
	  for (argc = 0; argc < MAX_ARGC; argc++)
	    argv[argc] = argvals[argc];
	  argc = margc - 1;
	  for (i = 1; i < margc; i++)
	    strcpy(argvals[i - 1], margv[i]);
	  margc = 0;
	}
      else
	{
	  /* Get a command */
	  if (!Prompt_input("Command: ", buf, sizeof(buf)))
	    {
	      if (cur_ms == NULLMS && feof(stdin))
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
	}
      if ((line = atoi(argv[0])) > 0 && line <= m->m_length)
	command = &m->m_lines[line - 1];
      else if ((!is_topmenu &&
		(!strcmp(argv[0], "r") || !strcmp(argv[0], "return"))) ||
	       !strcmp(argv[0], "q") || !strcmp(argv[0], "quit"))
	{
	  /* here if it's either return or quit */
#ifdef HAVE_CURSES
	  if (cur_ms != NULLMS)
	    {
	      cur_ms = old_cur_ms;
	      destroy_ms(my_ms);
	    }
#endif /* HAVE_CURSES */
	  if (m->m_exit != NULLFUNC)
	    m->m_exit(m);
	  return *argv[0] == 'r' ? DM_NORMAL : DM_QUIT;
	}
      else if (argv[0][0] == '?')
	{
	  for (line = 0; line < m->m_length; line++)
	    {
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
      else if (!strcmp(argv[0], "t") || !strcmp(argv[0], "toggle"))
	{
	  toggle_logging(argc, argv);
	  continue;
	}
      /* finally, try to find it using Find_command */
      else if (!(command = Find_command(m, argvals[0])))
	{
	  sprintf(buf, "Command not recognized: %s\n", argvals[0]);
	  Put_message(buf);
	  continue;
	}
      /* If we got to here, command is a valid menu_line */
      /* Send the offical command name into the argv */
      strcpy(argvals[0], command->ml_command);
      /* Show that we're working on it */
      Put_message(command->ml_doc);
      /* Print args that we've already got */
      for (i = 1; i < argc; i++)
	{
	  if (!command->ml_args[i].ma_prompt)
	    break;
	  sprintf(buf, "%s%s", command->ml_args[i].ma_prompt, argv[i]);
	  Put_message(buf);
	}
      /* Get remaining arguments, if any */
      for (; argc < command->ml_argc; argc++)
	{
	  if (!Prompt_input(command->ml_args[argc].ma_prompt,
			    argvals[argc], sizeof(argvals[argc])))
	    goto punt_command;
	}
      parsed_argc = argc - command->ml_argc;
      parsed_argv = &(argv[command->ml_argc]);
      if (command->ml_function != NULLFUNC)
	{
	  /* If it's got a function, call it */
	  quitflag = command->ml_function(argc, argv);
	}
      else if (command->ml_submenu != NULLMENU)
	{
	  /* Else see if it is a submenu */
	  quitflag = Do_menu(command->ml_submenu, argc, argv);
	}
      else
	{
	  /* If it's got neither, something is wrong */
	  Put_message("*INTERNAL ERROR: NO FUNCTION OR MENU FOR LINE*");
	}
      if (quitflag == DM_QUIT)
	{
#ifdef HAVE_CURSES
	  if (cur_ms != NULLMS)
	    {
	      cur_ms = old_cur_ms;
	      destroy_ms(my_ms);
	    }
#endif /* HAVE_CURSES */
	  if (m->m_exit != NULLFUNC)
	    m->m_exit(m);
	  parsed_argc = 0;
	  return DM_QUIT;
	}
    punt_command:
      parsed_argc = 0;
    }
}

void refresh_screen(void)
{
#ifdef HAVE_CURSES
  if (cur_ms != NULLMS)
    {
      touchwin(cur_ms->ms_screen);
      refresh_ms(cur_ms);
    }
#endif /* HAVE_CURSES */
}


/* Prompt the user for input in the input window of cur_ms */
int Prompt_input(char *prompt, char *buf, int buflen)
{
#ifdef HAVE_CURSES
  int c;
  char *p;
  int y, x, oldx, oldy;

  if (cur_ms != NULLMS)
    {
      more_flg = 1;
      getyx(cur_ms->ms_input, y, x);
      wmove(cur_ms->ms_input, y, 0);

      refresh_screen();
      waddstr(cur_ms->ms_input, prompt);
      getyx(cur_ms->ms_input, y, x);

      oldx = x;
      oldy = y;
      p = buf;
      while (1)
	{
	  wmove(cur_ms->ms_input, y, x);
	  touchwin(cur_ms->ms_screen);
	  wclrtoeol(cur_ms->ms_input);
	  wrefresh(cur_ms->ms_input);
	  c = getchar() & 0x7f;
	  switch (c)
	    {
	    case CTL('C'):
	      *p = '\0';
	      return 0;
	    case CTL('Z'):
	      kill(getpid(), SIGTSTP);
	      touchwin(cur_ms->ms_screen);
	      break;
	    case CTL('L'):
	      wclear(cur_ms->ms_input);
	      wmove(cur_ms->ms_input, 0, 0);
	      waddstr(cur_ms->ms_input, prompt);
	      touchwin(cur_ms->ms_input);
	      wrefresh(cur_ms->ms_screen);
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
	      if (p > buf)
		{
		  p--;
		  x--;
		  if (x < 0)
		    {
		      wmove(cur_ms->ms_input, y, 0);
		      wclrtoeol(cur_ms->ms_input);
		      y--;
		      x = getmaxx(cur_ms->ms_input) - 1;
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
	      if (isprint(c) && (p - buf < buflen - 1))
		{
		  waddch(cur_ms->ms_input, c);
		  *p++ = c;
		  x++;
		  if (x >= getmaxx(cur_ms->ms_input))
		    {
		      x = 0;
		      y++;
		    }
		}
	      else
		putchar(CTL('G'));
	      break;
	    }
	}
    end_input:
      waddch(cur_ms->ms_input, '\n');

      wclrtoeol(cur_ms->ms_input);
      refresh_ms(cur_ms);
      *p = '\0';
      Start_paging();
      strcpy(buf, strtrim(buf));
      return 1;
    }
  else
#endif /* HAVE_CURSES */
    {
      char bigbuf[BUFSIZ];

      printf("%s", prompt);
      if (!fgets(bigbuf, BUFSIZ, stdin))
	return 0;
      if (interrupt)
	{
	  interrupt = 0;
	  return 0;
	}
      Start_paging();
      strncpy(buf, strtrim(bigbuf), buflen);
      if (strchr(buf, '\n'))
	*strchr(buf, '\n') = '\0';
      else
	buf[buflen - 1] = '\0';
      return 1;
    }
}

int lines_left;

/* Start paging */
/* This routine will cause the most recently put message to be the
   one at the top of the screen when a ---More--- prompt is displayed */
void Start_paging(void)
{
#ifdef HAVE_CURSES
  if (cur_ms != NULLMS)
    lines_left = LINES - cur_ms->ms_input_y - 1;
  else
#endif /* HAVE_CURSES */
    lines_left = 23;
}

/* Turn off paging */
void Stop_paging(void)
{
  lines_left = -1;
}

/* Print a message in the input window of cur_ms.  */
void Put_message(char *msg)
{
  char *copy, *line, *s;

  copy = malloc(COLS);
  s = line = msg;
  if (log_file)
    {
      /* if we're doing logging; we assume that the file has already
	 been opened. */
      fprintf(log_file, "%s\n", msg);
      fflush(log_file);
    }

  while (*s++)
    {
      if (s - line >= COLS - 1)
	{
	  strncpy(copy, line, COLS - 1);
	  copy[COLS - 1] = '\0';
	  line += COLS - 1;
	}
      else if (*s == '\n')
	{
	  *s = '\0';
	  strcpy(copy, line);
	  line = ++s;
	}
      else
	continue;
      Put_line(copy);
    }
  Put_line(line);
  free(copy);
}

/* Will be truncated to COLS characters.  */
void Put_line(char *msg)
{
  if (!more_flg)
    return;

  if (lines_left >= 0)
    {
      if (--lines_left == 0)
	{
	  /* Give the user a more prompt */
#ifdef HAVE_CURSES
	  if (cur_ms != NULLMS)
	    {
              int x, y;
              char chr;

	      wstandout(cur_ms->ms_input);
	      wprintw(cur_ms->ms_input, "---More---");
	      wstandend(cur_ms->ms_input);
	      refresh_ms(cur_ms);
	      chr = getchar() & 0x7f;/* We do care what it is */
	      if (chr == 'q' || chr == 'Q' || chr == 3 /* ^C */)
		{
		  more_flg = 0;
		  return;
		}
	      getyx(cur_ms->ms_input, y, x);
	      /* x is a bitbucket; avoid lint problems */
	      x = x;
	      wmove(cur_ms->ms_input, y, 0);
	      wclrtoeol(cur_ms->ms_input);
	    }
	  else
#endif /* HAVE_CURSES */
	    {
	      printf("---More (hit return)---");
	      getchar();
	    }
	  Start_paging();	/* Reset lines_left */
	}
    }

#ifdef HAVE_CURSES
  if (cur_ms != NULLMS)
    {
      int i;
      char *msg1;

      msg1 = calloc(COLS, 1);
      strncpy(msg1, msg, COLS - 1);
      for (i = strlen(msg1); i < COLS - 1; i++)
	msg1[i] = ' ';
      wprintw(cur_ms->ms_input, "%s\n", msg1);
    }
  else
#endif /* HAVE_CURSES */
    puts(msg);
}

#ifdef HAVE_CURSES
/* Refresh a menu_screen onto the real screen */
void refresh_ms(struct menu_screen *ms)
{
  wrefresh(ms->ms_title);
  wrefresh(ms->ms_menu);
  wrefresh(ms->ms_input);
}
#endif /* HAVE_CURSES */

/* Parse buf into a list of words, which will be placed in strings specified by
   argv.  Space for these strings must have already been allocated.
   Only the first n characters of each word will be copied */
int Parse_words(char *buf, char *argv[], int n)
{
  char *start, *end;		/* For sausage machine */
  int argc;

  start = buf;
  for (argc = 0; argc < MAX_ARGC; argc++)
    {
      while (isspace(*start))
	start++;		/* Kill whitespace */
      if (*start == '\0')
	break;		/* Nothing left */
      /* Now find the end of the word */
      for (end = start; *end != '\0' && !isspace(*end); end++)
	;
      strncpy(argv[argc], start, MIN(end - start, n));	/* Copy it */
      argv[argc][MIN(end - start, n - 1)] = '\0';	/* Terminate */
      start = end;
    }
  return argc;
}

/* This is the internal form of Find_command, which recursively searches
   for a menu_line with command command in the specified menu */
/* It will search to a maximum depth of d */
struct menu_line *find_command_from(char *c, struct menu *m, int d)
{
  int line;
  struct menu_line *maybe;

  if (d < 0)
    return NULL;	/* Too deep! */
  for (line = 0; line < m->m_length; line++)
    {
      if (!strcmp(c, m->m_lines[line].ml_command))
	return &m->m_lines[line];
    }
  for (line = 0; line < m->m_length; line++)
    {
      if (m->m_lines[line].ml_submenu != NULLMENU &&
	  (maybe = find_command_from(c, m->m_lines[line].ml_submenu, d - 1)))
	return maybe;
    }
  /* If we got to here, nothing works */
  return NULL;
}

/* Find_command searches down the current menu tree */
/* And returns a pointer to a menu_line with the specified command name */
/* It returns (struct menu_line *) 0 if none is found */
struct menu_line *Find_command(Menu *m, char *command)
{
  if (m == NULLMENU)
    return NULL;
  else
    return find_command_from(command, m, MAX_MENU_DEPTH);
}

static char *get_tmp_dir(void)
{
#ifdef _WIN32
  static char tmp[BUFSIZ];
  DWORD len;
  if (!tmp[0])
    {
      len = GetTempPath(sizeof(tmp), tmp);
      if (!len || (len > sizeof(tmp)))
        strcpy(tmp, ".");
    }
  return tmp;
#else
  return "/var/tmp";
#endif
}

int toggle_logging(int argc, char *argv[])
{
  char buf[BUFSIZ];

  if (!log_file)
    {
      sprintf(buf, "%s/%s-log.%ld", get_tmp_dir(), whoami, (long)getpid());

      /* open the file */
      log_file = fopen(buf, "a");

      if (!log_file)
	Put_message("Open of log file failed.  Logging is not on.");
      else
	Put_message("Log file successfully opened.");
    }
  else
    { /* log_file is a valid pointer; turn off logging. */
      fflush(log_file);
      fclose(log_file);
      log_file = NULL;
      Put_message("Logging off.");
    }
  return DM_NORMAL;
}
