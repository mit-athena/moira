/* $Id: mailmaint.c,v 1.41 1998-05-26 18:13:40 danw Exp $
 *
 * Simple add-me-to/remove-me-from list client
 *
 *  mailmaint.c - pjlevine - 20 August 1987
 *
 * Copyright (C) 1988-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include <moira_site.h>

#include <ctype.h>
#ifdef HAVE_CURSES
#include <curses.h>
#endif
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <krb.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mailmaint/mailmaint.c,v 1.41 1998-05-26 18:13:40 danw Exp $");

#define STARTCOL 0
#define STARTROW 3
#define DISPROW 15
#define LISTMAX 50
#define LISTSIZE 32
#define CTL(ch) ((ch) & 037)
#define MAX(A, B) ((A) > (B) ? (A) : (B))

char *whoami;		/* should not be static, for logging package */
static int status;

typedef struct list_info {
  int active;
  int public;
  int hidden;
  int maillist;
  int group;
  char *acl_type;
  char *acl_name;
  char *desc;
  char *modtime;
  char *modby;
  char *modwith;
} List_info;

static char *ascbuff = {"0123456789"};
static List_info *current_li = (List_info *) NULL;

typedef struct _menu {
  int num_items;
  char *title;
  char **items;
} MENU;

MENU *main_menu, *help_menu;

int position[2], oldpos[2];
int level, found_some, currow, page, num_members;
int moreflg, toggle, first_time;
char *username;

void get_main_input(void);
void show_list_info(void);
void display_buff(char *buf);
void start_display_buff(char *buff);
void add_member(void);
void delete_member(void);
void list_by_member(void);
void show_all(void);
static int print_1(int argc, char *argv[], void *callback);
static int print_all(int argc, char *argv[], void *callback);
void list_all_groups(void);
void list_members(void);
static int print_2(int argc, char *argv[], void *callback);
void start_display(char *buff);
void end_display(void);
void display_menu(MENU *menu);
void pack_main_menu(void);
void pack_help_menu(void);
void highlight(MENU *menu);
void title(char *buff);
void center_text(int row, char *buff);
void show_text(int row, int col, char *buff);
void erase_line(int row, int col);
void cls(void);
void clrwin(int erase_row);
static int fetch_list_info(char *list, List_info *li);
static int get_list_info(int argc, char **argv, void *hint);
int Prompt(char *prompt, char *buf, int buflen, int crok);
void menu_err_hook(const char *who, long code, const char *fmt, va_list args);

/* This crock is because the original code was very broken and this makes
 * it work.  Someday, we should abandon the code or fix it right.
 */
#define mvcur(oy, ox, ny, nx) move(ny, nx)

/****************************************************/

int main(int argc, char *argv[])
{
  void (*old_hook)(const char *, long, const char *, va_list);
  int use_menu = 1, k_errno;
  char buf[BUFSIZ], pname[ANAME_SZ], *motd;

  if ((whoami = strrchr(argv[0], '/')) == NULL)
    whoami = argv[0];
  else
    whoami++;
  if (!(current_li = malloc(sizeof(List_info))))
    {
      sprintf(buf, ": allocating list info");
      goto punt;
    }
  else
    {
      current_li->acl_type = NULL;
      current_li->acl_name = NULL;
      current_li->desc = NULL;
      current_li->modtime = NULL;
      current_li->modby = NULL;
      current_li->modwith = NULL;
    }

  if ((k_errno = tf_init(TKT_FILE, R_TKT_FIL)) ||
      (k_errno = tf_get_pname(pname)))
    {
      com_err(whoami, k_errno, "reading Kerberos ticket file");
      exit(1);
    }
  tf_close();
  username = pname;

  printf("Connecting to database for %s...please hold on.\n", username);

  status = mr_connect(NULL);
  if (status)
    {
      sprintf(buf, "\nConnection to Moira server failed");
      goto punt;
    }

  status = mr_motd(&motd);
  if (status)
    {
      com_err(whoami, status, " unable to check server status");
      mr_disconnect();
      exit(2);
    }
  if (motd)
    {
      fprintf(stderr, "The Moira server is currently unavailable:\n%s\n",
	      motd);
      mr_disconnect();
      exit(2);
    }
  status = mr_auth("mailmaint");
  if (status)
    {
      sprintf(buf, "\nAuthorization failed.\n");
      goto punt;
    }

  initscr();
  if ((LINES < 24) || (COLS < 60))
    {
      display_buff("Display window too small.\n\n");
      sprintf(buf, "Current window parameters are (%d lines, %d columns)\n",
	      LINES, COLS);
      display_buff(buf);
      display_buff("Please resize your window\n");
      display_buff("to at least 24 lines and 60 columns.\n");
      exit(0);
    }
  raw();
  noecho();
  old_hook = set_com_err_hook(menu_err_hook);
  position[0] = oldpos[0] = 1;
  level = 0;
  pack_main_menu();
  pack_help_menu();
  display_menu(main_menu);
  get_main_input();
  cls();
  endwin();
  set_com_err_hook(old_hook);
  exit(0);

punt:
  com_err(whoami, status, buf);
  exit(1);
}

/****************************************************/
void get_main_input(void)
{
  int c;
  int retflg;

  while (1)
    {
      oldpos[level] = position[level];
      retflg = 0;
      currow = DISPROW + 2;
      page = 1;
      toggle = num_members = moreflg = 0;
      c = getchar() & 0x7f;	/* mask parity bit */
      if (c == '\r' || c == '\n')
	{
	  if (position[level] == 7)
	    c = 'q';
	  else
	    c = ascbuff[position[level]];
	  retflg = 1;
	}
      switch (c)
	{
	case 'L' & 037:	/* clear screen */
	  display_menu(main_menu);
	  break;
	case 'q':
	case 'Q':		/* quit */
	  position[level] = 7;
	  highlight(main_menu);
	  if (retflg)
	    {
	      cls();
	      return;
	    }
	  break;
	case '1':		/* show all lists */
	  position[level] = 1;
	  if (retflg)
	    show_all();
	  break;
	case '2':		/* get all members of a list */
	  position[level] = 2;
	  if (retflg)
	    list_members();
	  break;
	case '3':		/* display list which user is a recipient */
	  position[level] = 3;
	  if (retflg)
	    list_by_member();
	  break;
	case '4':		/* show description */
	  position[level] = 4;
	  if (retflg)
	    show_list_info();
	  break;
	case '5':		/* add to list */
	  position[level] = 5;
	  if (retflg)
	    add_member();
	  break;
	case '6':		/* delete */
	  position[level] = 6;
	  if (retflg)
	    delete_member();
	  break;
	case 27:		/* escape */
	  c = getchar() & 0x7f;
	  if (c == 91)
	    {
	      c = getchar() & 0x7f;
	      if (c == 65)	/* up arrow */
		{
		  position[level]--;
		  if (!position[level])
		    position[level] = 7;
		}
	      else
		{
		  if (c == 66)	/* down arrow */
		    {
		      position[level]++;
		      if (position[level] > 7)
			position[level] = 1;
		    }
		}
	    }
	  break;
	default:
	  printf("%c", 7);
	  break;
	}
      highlight(main_menu);
    }
}

/****************************************************/
void show_list_info(void)
{
  char *buf;

  show_text(DISPROW, STARTCOL, "Show information about a list.\n");
  buf = calloc(1024, 1);
  if (Prompt("Enter List Name: ", buf, LISTSIZE, 1) == 1)
    {
      display_buff("\n");
      if (fetch_list_info(buf, current_li) == 0)
	{
	  sprintf(buf, "Description: %s\n", current_li->desc);
	  if (strlen(buf) > 60)
	    display_buff(buf);
	  else
	    show_text(currow, STARTCOL, buf);
	  currow++;
	  sprintf(buf, "List Administrator: %s %s",
		  current_li->acl_type, current_li->acl_name);
	  show_text(currow, STARTCOL, buf);
	  currow++;
	  sprintf(buf, "Modified on %s by user %s with %s",
		  current_li->modtime, current_li->modby,
		  current_li->modwith);
	  show_text(currow, STARTCOL, buf);
	  currow++;
	}
      else
	{
	  show_text(currow, STARTCOL, "mailmaint: No such list found.");
	  currow++;
	}
      show_text(currow, STARTCOL, "Press any Key to continue...");
      getchar();
    }
  clrwin(DISPROW);
}

/****************************************************/
void display_buff(char *buf)
{
  int i, cnt;
  char *printbuf;
  int maxcol;

  maxcol = COLS;

  cnt = 0;
  printbuf = calloc(maxcol, 1);
  for (i = 0; i <= strlen(buf); i++)
    {
      printbuf[cnt] = buf[i];
      cnt++;
      if (cnt >= maxcol)
	{
	  start_display_buff(printbuf);
	  cnt = 0;
	  free(printbuf);
	  printbuf = calloc(maxcol, 1);
	}
    }
  if (strlen(buf) % maxcol != 0)
    {
      start_display_buff(printbuf);
      free(printbuf);
    }
  return;
}

/****************************************************/
void start_display_buff(char *buff)
{
  char buffer[5];

  num_members++;
  if (moreflg)
    return;
  if (currow >= LINES - 2)
    {
      page++;
      currow++;
      mvcur(0, 0, currow, STARTCOL);
      refresh();
      if (Prompt("--RETURN for more, ctl-c to exit--", buffer, 1, 0) == 0)
	{
	  erase_line(currow, STARTCOL);
	  show_text(currow, STARTCOL, "Flushing query...");
	  moreflg = 1;
	  return;
	}
      clrwin(DISPROW + 2);
      currow = DISPROW + 2;
      show_text(currow, STARTCOL, "continued");
      currow++;
    }
  show_text(currow, STARTCOL, buff);
  currow++;
  return;
}

/****************************************************/
void add_member(void)
{
  char *argv[3];
  char *buf;

  show_text(DISPROW, STARTCOL, "Add yourself to a list\n");
  buf = calloc(LISTMAX, 1);
  if (Prompt("Enter List Name: ", buf, LISTSIZE, 1) == 1)
    {
      display_buff("\n");
      argv[0] = strdup(buf);
      argv[1] = strdup("user");
      argv[2] = strdup(username);
      if ((status = mr_query("add_member_to_list", 3, argv, NULL, NULL)))
	{
	  display_buff("\n");
	  com_err(whoami, status, " found.\n");
	}
      else
	{
	  sprintf(buf, "User %s added to list\n", username);
	  show_text(DISPROW + 3, STARTCOL, buf);
	}
      currow = DISPROW + 4;
      show_text(DISPROW + 4, STARTCOL, "Press any Key to continue...");
      getchar();
    }
  clrwin(DISPROW);
}

/****************************************************/
void delete_member(void)
{
  char *argv[3];
  char *buf;

  show_text(DISPROW, STARTCOL, "Remove yourself from a list\n");
  buf = calloc(LISTMAX, 1);
  if (Prompt("Enter List Name: ", buf, LISTSIZE, 1) == 1)
    {
      display_buff("\n");
      argv[0] = strdup(buf);
      argv[1] = strdup("user");
      argv[2] = strdup(username);
      if ((status = mr_query("delete_member_from_list", 3, argv, NULL, NULL)))
	{
	  display_buff("\n");
	  com_err(whoami, status, " found.\n");
	}
      else
	{
	  sprintf(buf, "User %s deleted from list\n", username);
	  show_text(DISPROW + 3, STARTCOL, buf);
	}
      currow = DISPROW + 4;
      show_text(DISPROW + 4, STARTCOL, "Press any Key to continue...");
      getchar();
    }
  clrwin(DISPROW);
}

/****************************************************/
void list_by_member(void)
{
  char *nargv[3];
  char *buf;

  nargv[1] = strdup("ruser");
  nargv[2] = strdup(username);
  buf = calloc(BUFSIZ, 1);
  sprintf(buf, "%s is on the following lists:\n", username);
  show_text(DISPROW, STARTCOL, buf);
  mvcur(0, 0, currow, STARTCOL);
  refresh();
  if ((status = mr_query("get_lists_of_member", 2, nargv + 1, print_1, NULL)))
    {
      display_buff("\n");
      com_err(whoami, status, " in get_lists_of_member");
    }
  currow++;
  show_text(currow, STARTCOL, "Press any Key to continue...");
  getchar();
  clrwin(DISPROW);
}

/****************************************************/
void show_all(void)
{
  char c;

  show_text(DISPROW, STARTCOL, "This function may take a while... proceed? [n] ");
  c = getchar() & 0x7f;
  if (c == 'y' || c == 'Y')
    {
      move(DISPROW + 1, STARTCOL);
      addstr("Processing query...please hold");
      refresh();
      list_all_groups();
    }
  else
    erase_line(DISPROW, STARTCOL);
}

/****************************************************/
static int print_1(int argc, char *argv[], void *callback)
{
  char buf[BUFSIZ];

  /* no newline 'cause display_buff adds one */
  sprintf(buf, "%s\n", argv[0]);
  start_display(buf);

  return MR_CONT;
}

/****************************************************/
static int print_all(int argc, char *argv[], void *callback)
{
  char buf[BUFSIZ];

  if (moreflg)
    return 0;
  if (first_time)
    {
      erase_line(DISPROW + 1, STARTCOL);
      show_text(DISPROW + 1, STARTCOL, "All mailing lists:");
      first_time = 0;
    }
  sprintf(buf, "%s\n", argv[0]);
  start_display(buf);

  return MR_CONT;
}

/****************************************************/
void list_all_groups(void)
{
  char *argv[5];
  argv[0] = argv[1] = argv[3] = "true";
  argv[4] = "dontcare";
  argv[2] = "false";
  first_time = 1;
  if ((status = mr_query("qualified_get_lists", 5, argv, print_all, NULL)))
    {
      display_buff("\n");
      com_err(whoami, status, " in list_all_groups\n");
    }
  end_display();
}

/****************************************************/
void list_members(void)
{
  char *argv[1];
  char *buf;
  char buffer[80];

  found_some = 0;
  move(DISPROW, STARTCOL);
  mvcur(0, 0, DISPROW, STARTCOL);
  refresh();
  buf = calloc(LISTMAX, 1);
  if (Prompt("Enter List Name: ", buf, LISTSIZE, 1) == 1)
    {
      sprintf(buffer, "The members of list '%s' are:", buf);
      show_text(DISPROW + 1, STARTCOL, buffer);
      argv[0] = buf;
      if ((status = mr_query("get_members_of_list", 1, argv, print_2, NULL)))
	{
	  display_buff("\n");
	  com_err(whoami, status, " found.\n");
	  currow++;
	}
      if (!found_some)
	{
	  show_text(currow, STARTCOL, "List is empty (no members).");
	  currow++;
	  show_text(currow, STARTCOL, "Press any key to continue...");
	  getchar();
	  clrwin(DISPROW);
	  return;
	}
      end_display();
      return;
    }
  clrwin(DISPROW);
}

/****************************************************/
static int print_2(int argc, char *argv[], void *callback)
{
  char buf[BUFSIZ];

  found_some = 1;
  sprintf(buf, "%s %s", argv[0], argv[1]);
  start_display(buf);

  return MR_CONT;
}

/****************************************************/
void start_display(char *buff)
{
  char *buffer;
  int secondcol;   /* where to start the second column of text */

  secondcol = (COLS / 2);  /* 1/2 was accross the screen */
  num_members++;
  if (moreflg)
    return;
  buffer = calloc(50, 1);
  if (currow >= LINES - 2)
    {
      page++;
      mvcur(0, 0, currow, STARTCOL);
      refresh();
      if (Prompt("--RETURN for more, ctl-c to exit--", buffer, 1, 0) == 0)
	{
	  erase_line(currow, STARTCOL);
	  show_text(currow, STARTCOL, "Flushing query...");
	  moreflg = 1;
	  return;
	}
      clrwin(DISPROW + 2);
      currow = DISPROW + 2;
      sprintf(buffer, "Continued (Page %d)", page);
      show_text(currow, STARTCOL, buffer);
      currow++;
      toggle = 0;
    }
  if (!toggle)
    show_text(currow, STARTCOL, buff);
  else
    {
      erase_line(currow, secondcol - 1);  /* in case the 1st col is too long */
      show_text(currow, secondcol, buff);
      currow++;
    }
  toggle = !toggle;
}

/****************************************************/
void end_display(void)
{
  char *buffer;

  if (moreflg)
    {
      clrwin(DISPROW);
      return;
    }

  buffer = calloc(50, 1);
  currow++;
  sprintf(buffer, "End of List. %d Total Members\n", num_members - 1);
  show_text(currow, STARTCOL, buffer);
  currow++;
  show_text(currow, STARTCOL, "Press any key to continue...");
  getchar();
  clrwin(DISPROW);
}

/****************************************************/
void display_menu(MENU *menu)
{
  int i;

  cls();
  title(menu->title);
  mvcur(0, 0, STARTROW, STARTCOL);
  refresh();
  for (i = 0; i <= menu->num_items - 1; i++)
    {
      move(STARTROW + i, STARTCOL);
      standend();
      addstr(menu->items[i]);
      refresh();
    }
  center_text(STARTROW + menu->num_items + 2,
	      "Enter a number, <up arrow>, or <down arrow>.");
  if (!level)
    {
      center_text(STARTROW + menu->num_items + 3,
		  "Press 'q' to exit, <return> to confirm choice.");
    }
  else
    {
      center_text(STARTROW + menu->num_items + 3,
		  "Press 'q' to exit, 'r' for main menu, "
		  "<return> to confirm choice.");
    }

  if (!level)
    highlight(main_menu);
}

/****************************************************/
void pack_main_menu(void)
{
  char *buf;

  main_menu = malloc(sizeof(MENU));
  main_menu->num_items = 7;
  main_menu->items = malloc(sizeof(char *) * main_menu->num_items);

  buf = calloc(50, 1);
  sprintf(buf, "Mail List Program for %s", username);
  main_menu->title = strdup(buf);
  main_menu->items[0] = strdup("1.  Show all public mailing lists.");
  main_menu->items[1] = strdup("2.  Get all members of a mailing list.");
  main_menu->items[2] = strdup("3.  Display lists of which you are a member.");
  main_menu->items[3] = strdup("4.  Show description of list.");
  main_menu->items[4] = strdup("5.  Add yourself to a mailing list.");
  main_menu->items[5] = strdup("6.  Delete yourself from a mailing list.");
  main_menu->items[6] = strdup("q.  Quit.");
}

/****************************************************/
void pack_help_menu(void)
{
  help_menu = malloc(sizeof(MENU));
  help_menu->num_items = 5;
  help_menu->items = malloc(sizeof(char *) * help_menu->num_items);

  help_menu->title = strdup("mailmaint is designed as a basic mail list administration program.");
  help_menu->items[0] = strdup("if you need to perform more advanced list manipulation like");
  help_menu->items[1] = strdup("adding lists, or changing list characteristics, refer to the");
  help_menu->items[2] = strdup("program listmaint.");
  help_menu->items[3] = strdup(" ");
  help_menu->items[4] = strdup("Press any key to continue.");
}

/****************************************************/
void highlight(MENU *menu)
{
  if (oldpos[level] != position[level])
    {
      move(STARTROW + oldpos[level] - 1, STARTCOL);
      standend();
      addstr(menu->items[oldpos[level] - 1]);
      refresh();
    }

  move(STARTROW + position[level] - 1, STARTCOL);
  standout();
  addstr(menu->items[position[level] - 1]);
  refresh();
  standend();
  refresh();
}

/****************************************************/
void title(char *buff)
{
  move(0, MAX(0, (COLS - strlen(buff)) >> 1));
  standout();
  addstr(buff);
  refresh();
  standend();
}

/****************************************************/
void center_text(int row, char *buff)
{
  move(row, MAX(0, (COLS - strlen(buff)) >> 1));
  addstr(buff);
  refresh();
}

/****************************************************/
void show_text(int row, int col, char *buff)
{
  mvcur(0, 0, row, col);
  addstr(buff);
  refresh();
}

/****************************************************/
void erase_line(int row, int col)
{
  char *buff;
  int i;

  buff = calloc(COLS, 1);
  for (i = 0; i <= COLS - 2; i++)
    buff[i] = ' ';
  buff[i] = 0;		/* just to be sure ! */
  move(row, col);
  mvcur(0, 0, row, col);
  addstr(buff);
  refresh();
  free(buff);  /* close mem. leak */
}

/****************************************************/
void cls(void)
{
  clear();
  refresh();
}

/****************************************************/
void clrwin(int erase_row)
{
  int i;
  char *buff;
  int maxcol;

  maxcol = COLS;

  buff = calloc(maxcol + 1, 1);
  for (i = 0; i <= maxcol - 1; i++)
    buff[i] = ' ';
  buff[i] = 0;		/* just to be sure ! */
  mvcur(0, 0, erase_row, STARTCOL);
  refresh();
  for (i = erase_row; i <= currow - 1; i++)
    addstr(buff);
  addstr(buff);
  mvcur(erase_row, STARTCOL, STARTROW + oldpos[level] - 1, STARTCOL);
  refresh();
  free(buff);
}

/****************************************************/
static int fetch_list_info(char *list, List_info *li)
{
  char *argv[1];

  argv[0] = list;
  return mr_query("get_list_info", 1, argv, get_list_info, NULL);
}

static int get_list_info(int argc, char **argv, void *hint)
{
  if (current_li->acl_type)
    free(current_li->acl_type);
  current_li->acl_type = strdup(argv[7]);
  if (current_li->acl_name)
    free(current_li->acl_name);
  current_li->acl_name = strdup(argv[8]);
  if (current_li->desc)
    free(current_li->desc);
  current_li->desc = strdup(argv[9]);
  if (current_li->modtime)
    free(current_li->modtime);
  current_li->modtime = strdup(argv[10]);
  if (current_li->modby)
    free(current_li->modby);
  current_li->modby = strdup(argv[11]);
  if (current_li->modwith)
    free(current_li->modwith);
  current_li->modwith = strdup(argv[12]);
  return MR_CONT;
}


/****************************************************/
/* Prompt the user for input */
int Prompt(char *prompt, char *buf, int buflen, int crok)
{
  int c;
  char *p;

  addstr(prompt);
  refresh();
  for (p = buf; abs(strlen(p) - strlen(buf)) <= buflen;)
    {
      refresh();
      c = getchar() & 0x7f;
      switch (c)
	{
	case CTL('C'):
	  return 0;
	case CTL('Z'):
	  return 0;
	case CTL('L'):
	  cls();
	  display_menu(main_menu);
	  return 0;
	case '\n':
	case '\r':
	  if (crok)
	    display_buff("\n");
	  *p = '\0';
	  if (strlen(buf) < 1)	/* only \n or \r in buff */
	    return -1;
	  else
	    return 1;
	case '\b':
	case '\177':
	  if (p > buf)
	    {
	      p--;
	      printf("\b \b");
	    }
	  break;
	case CTL('U'):
	case CTL('G'):
	case CTL('['):
	  while (p-- > buf)
	    printf("\b \b");
	  p = buf;
	  break;
	default:
	  if (abs(strlen(p) - strlen(buf)) >= buflen)
	    {
	      printf("%c", 7);
	      break;
	    }
	  if (isprint(c))
	    {
	      addch(c);
	      *p++ = c;
	    }
	  else
	    putchar(CTL('G'));
	  break;
	}
    }
  return 0;
}


/*
 * Hook function to cause error messages to be printed through
 * curses instead of around it.
 */

void menu_err_hook(const char *who, long code, const char *fmt, va_list args)
{
  char buf[BUFSIZ], *cp;

  strcpy(buf, who);
  for (cp = buf; *cp; cp++)
    ;
  *cp++ = ':';
  *cp++ = ' ';
  if (code)
    {
      strcpy(cp, error_message(code));
      while (*cp)
	cp++;
    }
  vsprintf(cp, fmt, args);
  display_buff(buf);
}
