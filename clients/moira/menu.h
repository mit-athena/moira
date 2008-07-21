/* $Id: menu.h,v 1.5 1998-03-10 21:09:39 danw Exp $ */

#define MAX_ARGC 16		/* Maximum argument count per line */
#define MAX_ARGLEN 128		/* Maximum length of an argument */
#define MAX_LINES 16		/* Maximum number of lines per menu */

#define MAX_TITLE 1		/* Maximum length of title, in lines */

/* Maximum depth to which submenus will be searched when looking for commands */
#define MAX_MENU_DEPTH 8

typedef struct menu_arg {
  char *ma_doc;		/* Short doc for completion */
  char *ma_prompt;	/* For prompting in menu */
} Menu_Arg;

typedef struct menu_line {
  int (*ml_function)(int argc, char **argv);
  struct menu *ml_submenu;
  int ml_argc;
  struct menu_arg ml_args[MAX_ARGC];
} Menu_Line;

#define ml_doc ml_args[0].ma_prompt
#define ml_command ml_args[0].ma_doc

typedef struct menu {
  int (*m_entry)(struct menu *m, int argc, char **argv);
  int (*m_exit)(struct menu *m);
  char *m_title;
  int m_length;
  struct menu_line m_lines[MAX_LINES];
} Menu;

/* Return codes for Do_menu */
/* These should also be used by functions called from Do_menu */
#define DM_NORMAL 0		/* Normal return */
#define DM_QUIT 1		/* Quit; ^C or q command received */

/* Macros for initializing menu lines */
#define NULLFUNC (0)
#define NULLMENU NULL
#define SUBMENU(cmd, doc, menu) { NULLFUNC, menu, 1, { { cmd, doc } } }
#define SIMPLEFUNC(cmd, doc, func) { func, NULLMENU, 1, { { cmd, doc } } }

/* Unused words found on parsed command line */
extern int parsed_argc;
extern char **parsed_argv;

/* prototypes */
void Put_message(char *msg);
void Start_menu(Menu *m);
void Start_no_menu(Menu *m);
int Do_menu(Menu *m, int margc, char *margv[]);
void Cleanup_menu(void);
void Start_paging(void);
void Stop_paging(void);
int Prompt_input(char *prompt, char *buf, int buflen);
void refresh_screen(void);
