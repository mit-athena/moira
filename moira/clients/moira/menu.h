/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/menu.h,v 1.2 1990-04-09 18:02:53 mar Exp $ */

#define MAX_ARGC 16		/* Maximum argument count per line */
#define MAX_ARGLEN 128		/* Maximum length of an argument */
#define MAX_LINES 16		/* Maximum number of lines per menu */

#define MAX_TITLE 1		/* Maximum length of title, in lines */

/* Maximum depth to which submenus will be searched when looking for commands */
#define MAX_MENU_DEPTH 8

typedef struct menu_arg {
    char *ma_doc;		/* Short doc for completion */
    char *ma_prompt;		/* For prompting in menu */
}        Menu_Arg;

typedef struct menu_line {
    int (*ml_function) ();
    struct menu *ml_submenu;
    int ml_argc;
    struct menu_arg ml_args[MAX_ARGC];
}         Menu_Line;

#define ml_doc ml_args[0].ma_prompt
#define ml_command ml_args[0].ma_doc

typedef struct menu {
    int (*m_entry) ();
    int (*m_exit) ();
    char *m_title;
    int m_length;
    struct menu_line m_lines[MAX_LINES];
}    Menu;

/* Return codes for Do_menu */
/* These should also be used by functions called from Do_menu */
#define DM_NORMAL 0		/* Normal return */
#define DM_QUIT 1		/* Quit; ^C or q command received */

/* Macros for initializing menu lines */
#define NULLFUNC ((int (*)()) 0)
#define NULLMENU ((struct menu *) 0)
#define SUBMENU(cmd, doc, menu) { NULLFUNC, menu, 1, { { cmd, doc } } }
#define SIMPLEFUNC(cmd, doc, func) { func, NULLMENU, 1, { { cmd, doc } } }

/* Unused words found on parsed command line */
extern int parsed_argc;
extern char **parsed_argv;
