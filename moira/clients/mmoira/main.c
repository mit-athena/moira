/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mmoira/main.c,v 1.4 1992-10-13 11:15:25 mar Exp $
 *
 *  	Copyright 1991 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

#include	<mit-copyright.h>
#include	<stdio.h>
#include	<pwd.h>
#include	<moira.h>
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
	char *motd;
	int	n, status;
	char *getlogin();

	if ((user = getlogin()) == NULL)
	  user = getpwuid((int) getuid())->pw_name;
	user = (user && strlen(user)) ? strsave(user) : "";

	if ((program_name = rindex(argv[0], '/')) == NULL)
	  program_name = argv[0];
	else
	  program_name++;
	program_name = strsave(program_name);

	moira_server = "";
	for (n = 1; n < argc - 1; n++)
	  if (!strcmp(argv[n], "-db"))
	    moira_server = argv[n + 1];

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

	toplevel = XtInitialize("toplevel", "Moira", NULL, 0, 
				&argc, argv);

	XtAppAddActions(XtWidgetToApplicationContext(toplevel),
			actions, XtNumber(actions));

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


static char form_override_table[] =
    "None<Key>Return:	next-or-do-it()\n\
     Ctrl<Key>C:	cancel-form()\n\
     Shift<Key>Return:	execute-form()\n\
     Meta<Key>?:	help()";


DisplayForm(spec)
EntryForm	*spec;
{
	Widget	w;
	int i, j;
	static XtTranslations trans = NULL;

	if (trans == NULL)
	  trans = XtParseTranslationTable(form_override_table);

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

    XtAppNextEvent(_XtDefaultAppContext(), &event);
    XtDispatchEvent(&event);
}
