/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mailmaint/mailmaint.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mailmaint/mailmaint.c,v 1.25 1991-05-14 18:06:56 mar Exp $
 */

/*  (c) Copyright 1988 by the Massachusetts Institute of Technology. */
/*  For copying and distribution information, please see the file */
/*  <mit-copyright.h>. */

#ifndef lint
static char rcsid_mailmaint_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mailmaint/mailmaint.c,v 1.25 1991-05-14 18:06:56 mar Exp $";
#endif lint

/***********************************************************************/
/*  mailmaint.c - pjlevine - 20 August 1987 
    
*/
/***********************************************************************/
#include <stdio.h>
#include <pwd.h>
#include <signal.h>
#include <strings.h>
#include <curses.h>
#include <sys/types.h>
#include <varargs.h>
#include <com_err.h>
#include <ctype.h>
#include <moira.h>
#include <moira_site.h>
#include <mit-copyright.h>


#define STARTCOL 0
#define STARTROW 3
#define SECONDCOL 30
#define DISPROW 15
#define LISTMAX 50
#define LISTSIZE 32
#define CTL(ch)  ((ch) & 037)
#define MAX(A,B) ((A) > (B) ? (A) : (B))

char *whoami;		/* should not be static, for logging package */
static int status;
static int scream();
extern char *strsave();
#ifdef __STDC__
void menu_err_hook(const char *who, long code, const char *fmt, va_list args);
#else
void menu_err_hook();
#define const
#endif

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
}         List_info;

static char *ascbuff = {"0123456789"};
static int print_2(), print_1();
static List_info *current_li = (List_info *) NULL;
static int get_list_info();
static int fetch_list_info();

char *malloc();
char *getlogin();
extern char *strsave();
char *getenv();
char *calloc();

uid_t getuid();

typedef struct _menu {
    int num_items;
    char *title;
    char **items;
}     MENU;

MENU *main_menu, *help_menu;

int position[2], oldpos[2];
int level, found_some, currow, page, num_members;
int moreflg, toggle, first_time;
char *uname;


/****************************************************/
/*ARGSUSED*/
main(argc, argv)
    int argc;
    char *argv[];

{
#ifdef __STDC__
    void (*old_hook)(const char *, long, const char *, va_list);
#else
    void (*old_hook)();
#endif
    int use_menu = 1;
    char buf[BUFSIZ], *motd;

    if ((whoami = rindex(argv[0], '/')) == NULL)
	whoami = argv[0];
    else
	whoami++;
    uname = calloc(20, 1);
    if ((current_li = (List_info *) malloc(sizeof(List_info)))
	== (List_info *) NULL) {
	(void) sprintf(buf, ": allocating list info");
	goto punt;
    }
    else {
	current_li->acl_type = (char *) NULL;
	current_li->acl_name = (char *) NULL;
	current_li->desc = (char *) NULL;
	current_li->modtime = (char *) NULL;
	current_li->modby = (char *) NULL;
	current_li->modwith = (char *) NULL;
    }
    if ((uname = getlogin()) == NULL) {
	struct passwd *getpwuid();

	uname = getpwuid((int) getuid())->pw_name;
    }
    uname = (uname && strlen(uname)) ? strsave(uname) : "";

    printf("Connecting to database for %s...please hold on.\n", uname);

    status = mr_connect(NULL);
    if (status) {
	(void) sprintf(buf, "\nConnection to Moira server failed");
	goto punt;
    }

    status = mr_motd(&motd);
    if (status) {
        com_err(whoami, status, " unable to check server status");
	mr_disconnect();
	exit(2);
    }
    if (motd) {
	fprintf(stderr, "The Moira server is currently unavailable:\n%s\n", motd);
	mr_disconnect();
	exit(2);
    }
    status = mr_auth("mailmaint");
    if (status) {
	(void) sprintf(buf, "\nAuthorization failed.\n");
	goto punt;
    }

    if (use_menu) {
	(void) initscr();
	if ((LINES < 24) || (COLS < 60)) {
	    display_buff("Display window too small.\n\n");
	    (void) sprintf(buf, "Current window parameters are (%d \
lines, %d columns)\n", LINES, COLS);
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
    }
    exit(0);

punt:
    com_err(whoami, status, buf);
    exit(1);
}

/****************************************************/
get_main_input()
{
    int c;
    int retflg;

    while (1) {
	oldpos[level] = position[level];
	retflg = 0;
	currow = DISPROW + 2;
	page = 1;
	toggle = num_members = moreflg = 0;
	c = getchar() & 0x7f;	/* mask parity bit */
	if (c == 13) {
	    if (position[level] == 7)
		c = 'q';
	    else
		c = ascbuff[position[level]];
	    retflg = 1;
	}
	switch (c) {
	case 'L' & 037:	/* clear screen */
	    display_menu(main_menu);
	    break;
	case 'q':
	case 'Q':		/* quit */
	    position[level] = 7;
	    highlight(main_menu);
	    if (retflg) {
		cls();
		return;
	    }
	    break;
	case '1':		/* show all lists */
	    position[level] = 1;
	    if (retflg) {
		show_all();
	    }
	    break;
	case '2':		/* get all members of a list */
	    position[level] = 2;
	    if (retflg) {
		(void) list_members();
	    }
	    break;
	case '3':		/* display list which user is a recipient */
	    position[level] = 3;
	    if (retflg) {
		list_by_member();
	    }
	    break;
	case '4':		/* show description */
	    position[level] = 4;
	    if (retflg) {
		show_list_info();
	    }
	    break;
	case '5':		/* add to list */
	    position[level] = 5;
	    if (retflg) {
		add_member();
	    }
	    break;
	case '6':		/* delete */
	    position[level] = 6;
	    if (retflg) {
		delete_member();
	    }
	    break;
	case 27:		/* up arrow */
	    c = getchar() & 0x7f;
	    if (c == 91) {
		c = getchar() & 0x7f;
		if (c == 65) {
		    position[level]--;
		    if (!position[level])
			position[level] = 7;
		}
		else {
		    if (c == 66) {
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
show_list_info()
{
    char *buf;

    show_text(DISPROW, STARTCOL, "Show information about a list.\n\r");
    buf = calloc((unsigned)1024, 1);
    if (Prompt("Enter List Name: ", buf, LISTSIZE, 1) == 1) {
	display_buff("\n\r");
	if (fetch_list_info(buf, current_li) == 0) {
	    (void) sprintf(buf, "Description: %s\n\r", current_li->desc);
	    if (strlen(buf) > 60)
		(void) display_buff(buf);
	    else
		show_text(currow, STARTCOL, buf);
	    currow++;
	    (void) sprintf(buf, "List Administrator: %s %s",
			   current_li->acl_type, current_li->acl_name);
	    show_text(currow, STARTCOL, buf);
	    currow++;
	    (void) sprintf(buf, "Modified on %s by user %s with %s",
			   current_li->modtime, current_li->modby,
			   current_li->modwith);
	    show_text(currow, STARTCOL, buf);
	    currow++;
	}
	else {
	    show_text(currow, STARTCOL, "mailmaint: No such list found.");
	    currow++;
	}
	show_text(currow, STARTCOL, "Press any Key to continue...");
	(void) getchar();
    }
    clrwin(DISPROW);
}

/****************************************************/
display_buff(buf)
    char *buf;
{
    int i, cnt;
    char *printbuf;
    int maxcol;

    maxcol = COLS;
    if (maxcol >= 80)
	maxcol = 80;

    cnt = 0;
    printbuf = calloc((unsigned)maxcol, 1);
    for (i = 0; i <= strlen(buf); i++) {
	printbuf[cnt] = buf[i];
	cnt++;
	if (cnt >= maxcol) {
	    (void) start_display_buff(printbuf);
	    cnt = 0;
	    free(printbuf);
	    printbuf = calloc((unsigned)maxcol, 1);
	}
    }
    if (strlen(buf) % maxcol != 0) {
	(void) start_display_buff(printbuf);
	free(printbuf);
    }
    return (0);
}

/****************************************************/
start_display_buff(buff)
    char *buff;
{
    char buffer[5];

    num_members++;
    if (moreflg)
	return (0);
    if (currow >= LINES - 2) {
	page++;
	currow++;
	mvcur(0, 0, currow, STARTCOL);
	refresh();
	if (Prompt("--RETURN for more, ctl-c to exit--", buffer, 1, 0) == 0) {
	    display_buff("Flushing query...");
	    moreflg = 1;
	    return (0);
	}
	clrwin(DISPROW + 2);
	currow = DISPROW + 2;
	show_text(currow, STARTCOL, "continued");
	currow++;
    }
    show_text(currow, STARTCOL, buff);
    currow++;
    return (0);
}

/****************************************************/
add_member()
{
    char *argv[3];
    char *buf;

    show_text(DISPROW, STARTCOL, "Add yourself to a list\n\r");
    buf = calloc(LISTMAX, 1);
    if (Prompt("Enter List Name: ", buf, LISTSIZE, 1) == 1) {
	display_buff("\r\n");
	argv[0] = strsave(buf);
	argv[1] = strsave("user");
	argv[2] = strsave(uname);
	if (status = mr_query("add_member_to_list", 3, argv,
			       scream, (char *) NULL)) {
	    display_buff("\r\n");
	    com_err(whoami, status, " found.\n");
	}
	else {
	    (void) sprintf(buf, "User %s added to list\r", uname);
	    show_text(DISPROW + 3, STARTCOL, buf);
	}
	currow = DISPROW + 4;
	show_text(DISPROW + 4, STARTCOL, "Press any Key to continue...");
	(void) getchar();
    }
    clrwin(DISPROW);
}

/****************************************************/
delete_member()
{
    char *argv[3];
    char *buf;

    show_text(DISPROW, STARTCOL, "Remove yourself from a list\n\r");
    buf = calloc(LISTMAX, 1);
    if (Prompt("Enter List Name: ", buf, LISTSIZE, 1) == 1) {
	display_buff("\r\n");
	argv[0] = strsave(buf);
	argv[1] = strsave("user");
	argv[2] = strsave(uname);
	if (status = mr_query("delete_member_from_list", 3, argv,
			       scream, (char *) NULL)) {
	    display_buff("\r\n");
	    com_err(whoami, status, " found.\n");
	}
	else {
	    (void) sprintf(buf, "User %s deleted from list\r", uname);
	    show_text(DISPROW + 3, STARTCOL, buf);
	}
	currow = DISPROW + 4;
	show_text(DISPROW + 4, STARTCOL, "Press any Key to continue...");
	(void) getchar();
    }
    clrwin(DISPROW);
}

/****************************************************/
list_by_member()
{
    char *nargv[3];
    char *buf;

    nargv[1] = strsave("ruser");
    nargv[2] = strsave(uname);
    buf = calloc(BUFSIZ, 1);
    (void) sprintf(buf, "%s is on the following lists:\r", uname);
    show_text(DISPROW, STARTCOL, buf);
    mvcur(0, 0, currow, STARTCOL);
    refresh();
    if (status = mr_query("get_lists_of_member", 2, nargv + 1,
			   print_1, (char *) NULL)) {
	display_buff("\r\n");
	com_err(whoami, status, " in get_lists_of_member");
    }
    currow++;
    show_text(currow, STARTCOL, "Press any Key to continue...");
    (void) getchar();
    clrwin(DISPROW);
    return;
}

/****************************************************/
show_all()
{
    char c;

    show_text(DISPROW, STARTCOL, "This function may take a \
while... proceed? [y] ");
    c = getchar() & 0x7f;
    if (c == 'y' || c == 'Y' || c == '\n') {
	move(DISPROW + 1, STARTCOL);
	addstr("Processing query...please hold");
	refresh();
	(void) list_all_groups();
    }
    else
	erase_line(DISPROW, STARTCOL);
    return;

}

/****************************************************/
/*ARGSUSED*/
static int
print_1(argc, argv, callback)
    int argc;
    char *argv[], *callback;
{
    char buf[BUFSIZ];

    /* no newline 'cause display_buff adds one */
    (void) sprintf(buf, "%s\r", argv[0]);
    (void) start_display(buf);

    return (0);
}

/****************************************************/
/*ARGSUSED*/
static int
print_all(argc, argv, callback)
    int argc;
    char *argv[], *callback;
{
    char buf[BUFSIZ];

    if (moreflg)
	return (0);
    if (first_time) {
	erase_line(DISPROW + 1, STARTCOL);
	show_text(DISPROW + 1, STARTCOL, "All mailing lists:");
	first_time = 0;
    }
    (void) sprintf(buf, "%s\r", argv[0]);
    (void) start_display(buf);

    return (0);
}

/****************************************************/
list_all_groups()
{
    char *argv[5];
    argv[0] = argv[3] = "true";
    argv[1] = argv[4] = "dontcare";
    argv[2] = "false";
    first_time = 1;
    if (status = mr_query("qualified_get_lists", 5, argv,
			   print_all, (char *) NULL)) {
	display_buff("\r\n");
	com_err(whoami, status, " in list_all_groups\n");
    }
    end_display();

    return (0);
}

/****************************************************/
list_members()
{
    char *argv[1];
    char *buf;
    char buffer[80];

    found_some = 0;
    move(DISPROW, STARTCOL);
    mvcur(0, 0, DISPROW, STARTCOL);
    refresh();
    buf = calloc(LISTMAX, 1);
    if (Prompt("Enter List Name: ", buf, LISTSIZE, 1) == 1) {
	(void) sprintf(buffer, "The members of list '%s' are:", buf);
	show_text(DISPROW + 1, STARTCOL, buffer);
	argv[0] = buf;
	if (status = mr_query("get_members_of_list", 1, argv,
			       print_2, (char *) NULL)) {
	    display_buff("\r\n");
	    com_err(whoami, status, " found.\n");
	    currow++;
	}
	if (!found_some) {
	    show_text(currow, STARTCOL, "List is empty (no members).");
	    currow++;
	    show_text(currow, STARTCOL, "Press any key to continue...");
	    getchar();
	    clrwin(DISPROW);
	    return;
	}
	end_display();
	return(0);
    }
    clrwin(DISPROW);
    return (0);
}

/****************************************************/
/*ARGSUSED*/
static int
print_2(argc, argv, callback)
    int argc;
    char *argv[], *callback;
{
    char buf[BUFSIZ];

    found_some = 1;
    (void) sprintf(buf, "%s %s", argv[0], argv[1]);
    (void) start_display(buf);

    return (0);
}

/****************************************************/
start_display(buff)
    char *buff;
{
    char *buffer;

    num_members++;
    if (moreflg)
	return(0);
    buffer = calloc(50, 1);
    if (currow >= LINES - 2) {
	page++;
	currow++;
	mvcur(0, 0, currow, STARTCOL);
	refresh();
	if (Prompt("--RETURN for more, ctl-c to exit--", buffer, 1, 0) == 0) {
	    display_buff("Flushing query...");
	    moreflg = 1;
	    return (0);
	}
	clrwin(DISPROW + 2);
	currow = DISPROW + 2;
	(void) sprintf(buffer, "Continued (Page %d)", page);
	show_text(currow, STARTCOL, buffer);
	currow++;
	toggle = 0;
    }
    if (!toggle)
	show_text(currow, STARTCOL, buff);
    else {
	show_text(currow, SECONDCOL, buff);
	currow++;
    }
    toggle = !toggle;
    return(0);
}

/****************************************************/
end_display()
{
    char *buffer;

    if (moreflg) {
	clrwin(DISPROW);
	return;
    }

    buffer = calloc(50, 1);
    currow++;
    (void) sprintf(buffer, "End of List. %d Total Members\r", num_members);
    show_text(currow, STARTCOL, buffer);
    currow++;
    show_text(currow, STARTCOL, "Press any key to continue...");
    (void) getchar();
    clrwin(DISPROW);

}

/****************************************************/
display_menu(menu)
    MENU *menu;
{
    int i;

    cls();
    title(menu->title);
    mvcur(0, 0, STARTROW, STARTCOL);
    refresh();
    for (i = 0; i <= menu->num_items - 1; i++) {
	move(STARTROW + i, STARTCOL);
	standend();
	addstr(menu->items[i]);
	refresh();
    }
    center_text(STARTROW + menu->num_items + 2,
		"Enter a number, <up arrow>, or <down arrow>.");
    if (!level)
	center_text(STARTROW + menu->num_items + 3,
		    "Press 'q' to exit, <return> to confirm choice.");
    else
	center_text(STARTROW + menu->num_items + 3,
	"Press 'q' to exit, 'r' for main menu, <return> to confirm choice.");

    if (!level)
	highlight(main_menu);
}

/****************************************************/
pack_main_menu()
{
    char *buf;

    main_menu = (MENU *) malloc((unsigned) sizeof(MENU));
    main_menu->num_items = 7;
    main_menu->items = (char **) malloc((unsigned) sizeof(char *) * main_menu->num_items);

    buf = calloc(50, 1);
    (void) sprintf(buf, "Mail List Program for %s", uname);
    main_menu->title = strsave(buf);
    main_menu->items[0] = strsave("1.  Show all mailing lists.");
    main_menu->items[1] = strsave("2.  Get all members of a mailing list.");
    main_menu->items[2] = strsave("3.  Display lists of which you are a member.");
    main_menu->items[3] = strsave("4.  Show description of list.");
    main_menu->items[4] = strsave("5.  Add yourself to a mailing list.");
    main_menu->items[5] = strsave("6.  Delete yourself from a mailing list.");
    main_menu->items[6] = strsave("q.  Quit.");
}

/****************************************************/
pack_help_menu()
{
    help_menu = (MENU *) malloc((unsigned) sizeof(MENU));
    help_menu->num_items = 5;
    help_menu->items = (char **) malloc((unsigned) sizeof(char *) * help_menu->num_items);

    help_menu->title = strsave("mailmaint is designed as a basic mail list administration program.");
    help_menu->items[0] = strsave("if you need to perform more advanced list manipulation like");
    help_menu->items[1] = strsave("adding lists, or changing list characteristics, refer to the");
    help_menu->items[2] = strsave("program listmaint.");
    help_menu->items[3] = strsave(" ");
    help_menu->items[4] = strsave("Press any key to continue.");
}

/****************************************************/
highlight(menu)
    MENU *menu;
{


    if (oldpos[level] != position[level]) {
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
title(buff)
    char *buff;
{
    move(0, MAX(0, (COLS - strlen(buff)) >> 1));
    standout();
    addstr(buff);
    refresh();
    standend();
}

/****************************************************/
center_text(row, buff)
    int row;
    char *buff;
{
    move(row, MAX(0, (COLS - strlen(buff)) >> 1));
    addstr(buff);
    refresh();
}

/****************************************************/
show_text(row, col, buff)
    int row, col;
    char *buff;
{
    mvcur(0, 0, row, col);
    refresh();
    printf("%s", buff);
}

/****************************************************/
erase_line(row, col)
    int row, col;
{
    char *buff;
    int i;

    buff = calloc((unsigned)COLS, 1);
    for (i = 0; i <= COLS - 2; i++)
	buff[i] = ' ';
    buff[i] = 0;		/* just to be sure ! */
    move(row, col);
    mvcur(0, 0, row, col);
    printf("%s", buff);
    refresh();
}

/****************************************************/
cls()
{
    clear();
    refresh();
}

/****************************************************/
clrwin(erase_row)
    int erase_row;
{
    int i;
    char *buff;
    int maxcol;

    maxcol = COLS;
    if (maxcol > 80)
	maxcol = 80;		/* limit width */

    buff = calloc((unsigned)maxcol + 1, 1);
    for (i = 0; i <= maxcol - 1; i++)
	buff[i] = ' ';
    buff[i] = 0;		/* just to be sure ! */
    mvcur(0, 0, erase_row, STARTCOL);
    refresh();
    for (i = erase_row; i <= currow - 1; i++) {
	printf("%s\n\r", buff);
    }
    printf("%s", buff);
    mvcur(erase_row, STARTCOL, STARTROW + oldpos[level] - 1, STARTCOL);
    refresh();
}

/****************************************************/

static int
scream()
{
    com_err(whoami, status, "\nA Moira update returned a value -- programmer \
botch\n");
    mr_disconnect();
    exit(1);
    return(0);	/* to keep compiler happy */
}

/****************************************************/
/*ARGSUSED*/
static int 
fetch_list_info(list, li)
    char *list;
    List_info *li;
{
    char *argv[1];

    argv[0] = list;
    return mr_query("get_list_info", 1, argv, get_list_info, (char *) NULL);
}

/* ARGSUSED */
static int 
get_list_info(argc, argv)
    int argc;
    char **argv;
{

    if (current_li->acl_type)
	free(current_li->acl_type);
    current_li->acl_type = strsave(argv[7]);
    if (current_li->acl_name)
	free(current_li->acl_name);
    current_li->acl_name = strsave(argv[8]);
    if (current_li->desc)
	free(current_li->desc);
    current_li->desc = strsave(argv[9]);
    if (current_li->modtime)
	free(current_li->modtime);
    current_li->modtime = strsave(argv[10]);
    if (current_li->modby)
	free(current_li->modby);
    current_li->modby = strsave(argv[11]);
    if (current_li->modwith)
	free(current_li->modwith);
    current_li->modwith = strsave(argv[12]);
    return (0);
}




/****************************************************/
/* Prompt the user for input */
int 
Prompt(prompt, buf, buflen, crok)
    char *prompt;
    char *buf;
    int buflen;
    int crok;
{
    int c;
    char *p;

    printf("%s", prompt);
    refresh();
    for (p = buf; abs(strlen(p) - strlen(buf)) <= buflen;) {
	refresh();
	c = getchar() & 0x7f;
	switch (c) {
	case CTL('C'):
	    return 0;
	case CTL('Z'):
	    return 0;
	case CTL('L'):
	    cls();
	    display_menu(main_menu);
	    return (0);
	case '\n':
	case '\r':
	    if (crok)
		display_buff("\r");
	    *p = '\0';
	    if (strlen(buf) < 1)/* only \n or \r in buff */
		return (-1);
	    else
		return (1);
	case '\b':
	case '\177':
	    if (p > buf) {
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
	    if (abs(strlen(p) - strlen(buf)) >= buflen) {
		printf("%c", 7);
		break;
	    }
	    if (isprint(c)) {
		(void) addch(c);
		*p++ = c;
	    }
	    else
		(void) putchar(CTL('G'));
	    break;
	}
    }
    return(0);
}


/*
 * Hook function to cause error messages to be printed through
 * curses instead of around it.
 */

void
menu_err_hook(who, code, fmt, args)
    const char *who;
    long code;
    const char *fmt;
    va_list args;
{
    char buf[BUFSIZ], *cp;

    (void) strcpy(buf, who);
    for (cp = buf; *cp; cp++);
    *cp++ = ':';
    *cp++ = ' ';
    if (code) {
	(void) strcpy(cp, error_message(code));
	while (*cp)
	    cp++;
    }
#if defined(AIX386) || defined(sun)
    vsprintf(cp, fmt, args);
#else
    /* can do this because we never pass more than 1 arg here anyway... */
    sprintf(cp, fmt, args);
#endif
    display_buff(buf);
}
