/*
** Buttons & Pulldown menus across the top of the application.
** When selected, it will continue to the submenu if non-null,
** otherwise call the general callback.
*/

typedef struct MenuItemS {
	char		*label;		/* the displayed label */
	struct MenuItemS **submenu;	/* if non-null, a submenu to display */
	int		operation;	/* op identifer */
	char		*form;		/* name of form to popup */
	char		*query;
	int		argc;
} MenuItem;


typedef struct {
	char	*prompt;		/* text displayed to left of input */
	int	type;			/* what sort of input expected */
	union {				/* default value / return value */
		char	*stringvalue;
		Boolean	booleanvalue;
		int	integervalue;
	} returnvalue;
	Boolean	insensitive;		/* grayed-out or not... */
	int	(*valuechanged)();	/* if non-NULL, call */
	char	**keywords;		/* list of possible values */
	Widget	mywidget;
} UserPrompt;

/* field types */
#define FT_STRING	1
#define FT_BOOLEAN	2
#define FT_KEYWORD	3
#define FT_NUMBER	4

/* max length of character fields */
#define MAXFIELDSIZE	256

#define stringval(f, n) ((f)->inputlines[n]->returnvalue.stringvalue)
#define boolval(f, n) ((f)->inputlines[n]->returnvalue.booleanvalue)

/*
** All (foo **) entries are assumed to be null-terminated lists of foos.
*/

typedef struct {
	char	*label;			/* text displayed on button */
	int	(*returnfunction) ();	/* function called when button hit */
} BottomButton;

/*
** Pass a pointer to the filled-in EntryForm structure back to the
** returnfunction when the button is hit.
*/

typedef struct {
	char		*formname;	/* name of form (duh!) */
	char		*instructions;	/* text at top of form */
	UserPrompt	**inputlines;	/* list of input lines */
	BottomButton	**buttons;	/* list of buttons on the bottom */
	Widget		formpointer;	/* already exists? */
	MenuItem	*menu;
	caddr_t		extrastuff;	/* for future expansion */
} EntryForm;

/*
** When you're coming up, check to see if formpointer is initialized.
** If so, map it.  Otherwise, create the form and set formpointer to
** the new toplevel widget id.  Then map it.
*/

