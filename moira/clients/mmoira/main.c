/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mmoira/main.c,v 1.8 1992-11-04 18:00:16 mar Exp $
 *
 *  	Copyright 1991 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

#include	<mit-copyright.h>
#include	<stdio.h>
#include	<strings.h>
#include	<pwd.h>
#include	<moira.h>
#include	<com_err.h>
#include	<X11/StringDefs.h>
#include	<Xm/PushB.h>
#include	<Xm/BulletinB.h>
#include	<Xm/RowColumn.h>
#include	<Xm/RowColumnP.h>
#include	"mmoira.h"

extern MenuItem MenuRoot;

Widget	CreateMenu(), CreateForm();
Widget	BuildMenuTree();
void popup_error_hook(), mr_x_input(), EnterPressed(), CancelForm();
void ExecuteForm(), DoHelp();

MoiraResources resources;

static XrmOptionDescRec options[] = {
    {"-db",	"*database",	XrmoptionSepArg,	NULL},
    {"-helpfile","*helpFile",	XrmoptionSepArg,	NULL},
};


#define Offset(field) (XtOffset(MoiraResources *, field))

static XtResource my_resources[] = {
    {"formTranslations", XtCString, XtRString, sizeof(String),
       Offset(form_trans), XtRImmediate, (caddr_t) NULL},
    {"textTranslations", XtCString, XtRString, sizeof(String),
       Offset(text_trans), XtRImmediate, (caddr_t) NULL},
    {"logTranslations", XtCString, XtRString, sizeof(String),
       Offset(log_trans), XtRImmediate, (caddr_t) NULL},
    {"helpFile", XtCFile, XtRString, sizeof(String),
       Offset(help_file), XtRImmediate, (caddr_t) "/afs/athena.mit.edu/system/moira/lib/mmoira.helpfile"},
    {"database", XtCString, XtRString, sizeof(String),
       Offset(db), XtRImmediate, (caddr_t) NULL},
};

#undef Offset

XtActionsRec actions[] = {
    { "next-or-do-it", EnterPressed },
    { "cancel-form", CancelForm },
    { "execute-form", ExecuteForm },
    { "help", DoHelp }
};


Widget  toplevel;
char *user, *program_name, *moira_server;

main(argc, argv)
int argc;
char *argv[];
{
	Widget	button, bboard, menuwidget;
	char *motd, *env;
	int	n, status;
	char *getlogin(), *getenv();

	/* I know, this is a crock, but it makes the program work... */
	env = getenv("XFILESEARCHPATH");
	if (env) {
	    char buf[256];
	    sprintf(buf, "%s:/afs/athena.mit.edu/system/moira/lib/%N", env);
	    setenv("XFILESEARCHPATH", buf, 1);
	} else
	  setenv("XFILESEARCHPATH", "/afs/athena.mit.edu/system/moira/lib/%N", 1);

	if ((user = getlogin()) == NULL)
	  user = getpwuid((int) getuid())->pw_name;
	user = (user && strlen(user)) ? strsave(user) : "";

	if ((program_name = rindex(argv[0], '/')) == NULL)
	  program_name = argv[0];
	else
	  program_name++;
	program_name = strsave(program_name);

	toplevel = XtInitialize("toplevel", "Moira", options,
				XtNumber(options), &argc, argv);

	XtAppAddActions(XtWidgetToApplicationContext(toplevel),
			actions, XtNumber(actions));

	XtGetApplicationResources(toplevel, (caddr_t) &resources, 
				  my_resources, XtNumber(my_resources),
				  NULL, (Cardinal) 0);

	moira_server = "";
	if (resources.db)
	  moira_server = resources.db;

#ifdef GDSS
	initialize_gdss_error_table();
#endif /* GDSS */

	status = mr_connect(moira_server);
	if (status) {
	    com_err(program_name, status, " connecting to server");
	    exit(1);
	}
	status = mr_motd(&motd);
	if (status) {
	    com_err(program_name, status, " connecting to server");
	    exit(1);
	}
	if (motd) {
	    fprintf(stderr, "The Moira server is currently unavailable:\n%s\n",
		    motd);
	    mr_disconnect();
	    exit(1);
	}

      	status = mr_auth("mmoira");
	if (status == MR_USER_AUTH) {
	    char buf[BUFSIZ];
	    com_err(program_name, status, "\nPress [RETURN] to continue");
	    gets(buf);
	} else if (status) {
	    com_err(program_name, status, "; authorization failed - may need to run kinit");
	    exit(1);
	}

	bboard = XtCreateManagedWidget(	"bboard",
				       xmBulletinBoardWidgetClass,
				       toplevel, NULL, 0);
	menuwidget = BuildMenuTree(bboard, &MenuRoot);
	SetupLogWidget(bboard);

	XtRealizeWidget(toplevel);

	set_com_err_hook(popup_error_hook);
	mr_set_alternate_input(ConnectionNumber(XtDisplay(toplevel)),
			       mr_x_input);
	XtMainLoop();
}


int MoiraQuery(query, argc, argv, callback, data)
char *query;
int argc;
char **argv;
int (*callback)();
caddr_t data;
{
    int status;

    MakeWatchCursor(toplevel);
    XFlush(XtDisplay(toplevel));
    status = mr_query(query, argc, argv, callback, data);
    if (status != MR_ABORTED && status != MR_NOT_CONNECTED) {
	MakeNormalCursor(toplevel);
	return(status);
    }
    status = mr_connect(moira_server);
    if (status) {
	com_err(program_name, status, " while re-connecting to server %s",
		moira_server);
	MakeNormalCursor(toplevel);
	return(MR_ABORTED);
    }
    status = mr_auth("mmoira");
    if (status) {
	com_err(program_name, status, " while re-authenticating to server %s",
		moira_server);
	mr_disconnect();
	MakeNormalCursor(toplevel);
	return(MR_ABORTED);
    }
    status = mr_query(query, argc, argv, callback, data);
    MakeNormalCursor(toplevel);
    return(status);

}


DisplayForm(spec)
EntryForm	*spec;
{
	Widget	w;
	int i, j;
	static XtTranslations trans = NULL;

	if (trans == NULL)
	  trans = XtParseTranslationTable(resources.form_trans);

	w = CreateForm(toplevel, spec);
	XtManageChild(w);
	for (i = 0; spec->inputlines[i]; i++) {
	    XtOverrideTranslations(spec->inputlines[i]->mywidget, trans);
	    if (spec->inputlines[i]->type == FT_KEYWORD) {
		XmRowColumnWidget rc;

		rc = (XmRowColumnWidget) spec->inputlines[i]->mywidget;
		for (j = 0; j < rc->composite.num_children; j++)
		  XtOverrideTranslations(rc->composite.children[j], trans);
	    }
	}
	/* set the focus to the first line of the form */
	_XmGrabTheFocus(spec->inputlines[0]->mywidget, NULL);
}


void popup_error_hook(who, code, fmt, arg1, arg2, arg3, arg4, arg5)
char *who;
long code;
char *fmt;
caddr_t arg1, arg2, arg3, arg4, arg5;
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
    sprintf(cp, fmt, arg1, arg2, arg3, arg4, arg5);
    display_error(buf);
}


void mr_x_input()
{
    XEvent event;
    XtAppContext _XtDefaultAppContext();

    XtAppNextEvent(_XtDefaultAppContext(), &event);
    XtDispatchEvent(&event);
}
