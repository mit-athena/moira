/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mmoira/main.c,v 1.2 1991-05-31 16:46:39 mar Exp $
 *
 *  	Copyright 1991 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

#include	<mit-copyright.h>
#include	<stdio.h>
#include	<moira.h>
#include	<Xm/PushB.h>
#include	<Xm/BulletinB.h>
#include	"mmoira.h"

extern MenuItem MenuRoot;

Widget	CreateMenu(), CreateForm();
Widget	BuildMenuTree();
void popup_error_hook();

Widget  toplevel;
char *user, *program_name, *moira_server;

main(argc, argv)
int argc;
char *argv[];
{
	Widget	button, bboard, menuwidget;
	int	n;

	user = "mar";
	program_name = argv[0];
	moira_server = "";

	mr_connect(moira_server);
	mr_auth("mmoira");

	toplevel = XtInitialize("toplevel", "Moira", NULL, 0, 
				&argc, argv);

	bboard = XtCreateManagedWidget(	"bboard",
				       xmBulletinBoardWidgetClass,
				       toplevel, NULL, 0);
	menuwidget = BuildMenuTree(bboard, &MenuRoot);
	SetupLogWidget(bboard);

	XtRealizeWidget(toplevel);

	set_com_err_hook(popup_error_hook);
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

    MakeWatchCursor();
    status = mr_query(query, argc, argv, callback, data);
    if (status != MR_ABORTED && status != MR_NOT_CONNECTED) {
	MakeNormalCursor();
	return(status);
    }
    status = mr_connect(moira_server);
    if (status) {
	com_err(program_name, status, " while re-connecting to server %s",
		moira_server);
	MakeNormalCursor();
	return(MR_ABORTED);
    }
    status = mr_auth("mmoira");
    if (status) {
	com_err(program_name, status, " while re-authenticating to server %s",
		moira_server);
	mr_disconnect();
	MakeNormalCursor();
	return(MR_ABORTED);
    }
    status = mr_query(query, argc, argv, callback, data);
    MakeNormalCursor();
    return(status);

}


DisplayForm(spec)
EntryForm	*spec;
{
	Widget	w;

	w = CreateForm(toplevel, spec);
	XtManageChild(w);
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
